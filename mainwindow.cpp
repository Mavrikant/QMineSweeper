#include "mainwindow.h"

#include "./ui_mainwindow.h"
#include "average_time_format.h"
#include "bv_per_second_format.h"
#include "flag_accuracy_format.h"
#include "language.h"
#include "safe_percent_format.h"
#include "stats.h"
#include "telemetry.h"
#include "time_format.h"
#include "tutorial.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFont>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QKeySequence>
#include <QLabel>
#include <QLayout>
#include <QLocale>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QTableWidget>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>

namespace
{
QString difficultyName(const Difficulty &diff)
{
    if (diff.width == MineField::Beginner.width && diff.height == MineField::Beginner.height && diff.mineCount == MineField::Beginner.mineCount)
    {
        return QStringLiteral("Beginner");
    }
    if (diff.width == MineField::Intermediate.width && diff.height == MineField::Intermediate.height && diff.mineCount == MineField::Intermediate.mineCount)
    {
        return QStringLiteral("Intermediate");
    }
    if (diff.width == MineField::Expert.width && diff.height == MineField::Expert.height && diff.mineCount == MineField::Expert.mineCount)
    {
        return QStringLiteral("Expert");
    }
    return QStringLiteral("Custom");
}
} // namespace

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(std::make_unique<Ui::MainWindow>()), m_releaseId(QStringLiteral("qminesweeper@") + QString::fromUtf8(QMS_VERSION))
{
    ui->setupUi(this);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    ui->mineFieldWidget->setMineCountLabel(ui->mineCount);

    // Load the per-user "question marks enabled" preference before the grid
    // sees any right-click. Default true preserves v1.4.x behaviour.
    {
        QSettings s;
        MineButton::setQuestionMarksEnabled(s.value(QStringLiteral("settings/question_marks"), true).toBool());
        // Default false — the classic palette is what players expect on a
        // first launch; the colour-blind-friendly palette is opt-in.
        MineButton::setColorBlindPaletteEnabled(s.value(QStringLiteral("settings/colorblind_palette"), false).toBool());
    }

    m_displayTimer = new QTimer(this);
    m_displayTimer->setInterval(50);
    connect(m_displayTimer, &QTimer::timeout, this, &MainWindow::updateTimerLabel);

    // Smiley button — clickable "new game" affordance and at-a-glance status
    // indicator. Reuses the existing "New Game" string so no new translation.
    ui->smileyButton->setToolTip(tr("New Game"));
    setSmileyState(GameState::Ready);
    connect(ui->smileyButton, &QPushButton::clicked, this, &MainWindow::onNewGame);

    buildMenus();

    QSettings settings;
    const QString lastDifficulty = settings.value("difficulty", difficultyName(MineField::Beginner)).toString();
    Difficulty startDifficulty = MineField::Beginner;
    if (lastDifficulty == QStringLiteral("Intermediate"))
    {
        startDifficulty = MineField::Intermediate;
    }
    else if (lastDifficulty == QStringLiteral("Expert"))
    {
        startDifficulty = MineField::Expert;
    }
    else if (lastDifficulty == QStringLiteral("Custom"))
    {
        // Restore the last custom geometry; fall back to Expert-sized defaults
        // if any key is missing or malformed. Clamp to the same ranges the
        // dialog enforces to guard against hand-edited plists.
        const int cw = std::clamp(settings.value(QStringLiteral("custom_width"), 30).toInt(), 9, 30);
        const int ch = std::clamp(settings.value(QStringLiteral("custom_height"), 16).toInt(), 9, 24);
        const int totalCells = cw * ch;
        const int cm = std::clamp(settings.value(QStringLiteral("custom_mines"), 99).toInt(), 10, std::max(10, totalCells - 9));
        startDifficulty = Difficulty{static_cast<std::uint32_t>(cw), static_cast<std::uint32_t>(ch), static_cast<std::uint32_t>(cm)};
        m_isCustom = true;
    }
    m_currentDifficulty = startDifficulty;

    // Tick the checkable difficulty action that matches the restored difficulty.
    const QString targetKey = m_isCustom ? QStringLiteral("Custom") : difficultyName(startDifficulty);
    const QList<QAction *> diffActions = m_difficultyGroup->actions();
    for (QAction *action : diffActions)
    {
        if (action->data().toString() == targetKey)
        {
            action->setChecked(true);
            break;
        }
    }

    ui->mineFieldWidget->newGame(m_currentDifficulty);

    connect(ui->mineFieldWidget, &MineField::gameStarted, this, &MainWindow::onGameStarted);
    connect(ui->mineFieldWidget, &MineField::gameWon, this, &MainWindow::onGameWon);
    connect(ui->mineFieldWidget, &MineField::gameLost, this, &MainWindow::onGameLost);
    connect(ui->mineFieldWidget, &MineField::mineCountChanged, this, [this](int remaining) { ui->mineCount->setNum(remaining); });
    connect(ui->mineFieldWidget, &MineField::cellInteractionStarted, this, [this] { setSmileyTension(true); });
    connect(ui->mineFieldWidget, &MineField::cellInteractionEnded, this, [this] { setSmileyTension(false); });

    resetTimerUi();
    setWindowTitle(tr("QMineSweeper"));

    centralWidget()->layout()->setContentsMargins(0, 0, 0, 0);
    adjustSize();
    setFixedSize(sizeHint());

    maybeAskTelemetryConsent();

    // First-run tutorial: show once on the initial launch for each install.
    // Deferred so the main window has a chance to paint underneath.
    if (!Tutorial::isCompleted())
    {
        QTimer::singleShot(0, this, &MainWindow::showTutorialDialog);
    }
}

MainWindow::~MainWindow() = default;

void MainWindow::buildMenus()
{
    auto *newAction = new QAction(tr("&New"), this);
    newAction->setShortcut(QKeySequence(QKeySequence::New));
    connect(newAction, &QAction::triggered, this, &MainWindow::onNewGame);
    ui->menuGame->addAction(newAction);

    m_replayAction = new QAction(tr("&Replay same layout"), this);
    m_replayAction->setShortcut(QKeySequence(QKeySequence::Refresh));
    m_replayAction->setEnabled(false);
    connect(m_replayAction, &QAction::triggered, this, &MainWindow::onReplaySameLayout);
    ui->menuGame->addAction(m_replayAction);

    m_pauseAction = new QAction(tr("&Pause"), this);
    m_pauseAction->setShortcut(QKeySequence(Qt::Key_P));
    m_pauseAction->setEnabled(false);
    connect(m_pauseAction, &QAction::triggered, this, &MainWindow::onTogglePause);
    ui->menuGame->addAction(m_pauseAction);

    ui->menuGame->addSeparator();

    auto *diffMenu = ui->menuGame->addMenu(tr("&Difficulty"));
    m_difficultyGroup = new QActionGroup(this);
    m_difficultyGroup->setExclusive(true);

    struct Entry
    {
        const char *label;
        Difficulty diff;
        const char *key;
    };
    // QT_TR_NOOP marks the literal for lupdate extraction; the runtime tr()
    // lookup at the use site then resolves it via the installed translator.
    const Entry entries[] = {
        {QT_TR_NOOP("&Beginner  (9×9, 10 mines)"), MineField::Beginner, "Beginner"},
        {QT_TR_NOOP("&Intermediate  (16×16, 40 mines)"), MineField::Intermediate, "Intermediate"},
        {QT_TR_NOOP("&Expert  (30×16, 99 mines)"), MineField::Expert, "Expert"},
    };
    for (const auto &e : entries)
    {
        auto *action = new QAction(tr(e.label), this);
        action->setCheckable(true);
        action->setData(QString::fromLatin1(e.key));
        m_difficultyGroup->addAction(action);
        diffMenu->addAction(action);
        const Difficulty d = e.diff;
        connect(action, &QAction::triggered, this, [this, d] { onDifficultyChanged(d); });
    }

    diffMenu->addSeparator();
    m_customDifficultyAction = new QAction(tr("&Custom…"), this);
    m_customDifficultyAction->setCheckable(true);
    m_customDifficultyAction->setData(QStringLiteral("Custom"));
    m_difficultyGroup->addAction(m_customDifficultyAction);
    diffMenu->addAction(m_customDifficultyAction);
    connect(m_customDifficultyAction, &QAction::triggered, this, &MainWindow::onDifficultyCustom);

    ui->menuGame->addSeparator();

    auto *statsAction = new QAction(tr("&Statistics…"), this);
    connect(statsAction, &QAction::triggered, this, &MainWindow::showStatsDialog);
    ui->menuGame->addAction(statsAction);

    ui->menuGame->addSeparator();

    auto *quitAction = new QAction(tr("&Quit"), this);
    quitAction->setShortcut(QKeySequence(QKeySequence::Quit));
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);
    ui->menuGame->addAction(quitAction);

    auto *settingsMenu = menuBar()->addMenu(tr("&Settings"));

    // Language submenu: flag-iconified radio group + "Auto (system)".
    auto *languageMenu = settingsMenu->addMenu(tr("&Language"));
    m_languageGroup = new QActionGroup(this);
    m_languageGroup->setExclusive(true);

    const QString currentOverride = Language::userOverride();
    for (const auto &entry : Language::supported())
    {
        auto *langAction = new QAction(entry.nativeName, this);
        langAction->setCheckable(true);
        langAction->setIcon(QIcon(entry.flagResource));
        langAction->setData(entry.code);
        langAction->setChecked(currentOverride == entry.code);
        m_languageGroup->addAction(langAction);
        languageMenu->addAction(langAction);
        const QString code = entry.code;
        connect(langAction, &QAction::triggered, this, [this, code] { onLanguageChosen(code); });
    }
    languageMenu->addSeparator();
    auto *autoAction = new QAction(tr("Auto (system)"), this);
    autoAction->setCheckable(true);
    autoAction->setChecked(currentOverride.isEmpty());
    m_languageGroup->addAction(autoAction);
    languageMenu->addAction(autoAction);
    connect(autoAction, &QAction::triggered, this, [this] { onLanguageChosen(QString()); });

    settingsMenu->addSeparator();
    m_questionMarksAction = new QAction(tr("Enable &question marks"), this);
    m_questionMarksAction->setCheckable(true);
    m_questionMarksAction->setChecked(MineButton::questionMarksEnabled());
    connect(m_questionMarksAction, &QAction::toggled, this, &MainWindow::toggleQuestionMarks);
    settingsMenu->addAction(m_questionMarksAction);

    m_colorBlindPaletteAction = new QAction(tr("&Color-blind friendly numbers"), this);
    m_colorBlindPaletteAction->setCheckable(true);
    m_colorBlindPaletteAction->setChecked(MineButton::colorBlindPaletteEnabled());
    connect(m_colorBlindPaletteAction, &QAction::toggled, this, &MainWindow::toggleColorBlindPalette);
    settingsMenu->addAction(m_colorBlindPaletteAction);

    if (Telemetry::isCompiledIn())
    {
        settingsMenu->addSeparator();
        m_telemetryAction = new QAction(tr("Send anonymous &crash reports and usage data"), this);
        m_telemetryAction->setCheckable(true);
        m_telemetryAction->setChecked(Telemetry::isEnabled());
        connect(m_telemetryAction, &QAction::toggled, this, &MainWindow::toggleTelemetry);
        settingsMenu->addAction(m_telemetryAction);
    }

    if (ui->actionAbout)
    {
        connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::showAboutDialog);
    }

    // Help → Tutorial (always re-openable, regardless of first-run flag).
    if (ui->menuHelp)
    {
        auto *tutorialAction = new QAction(tr("&Tutorial"), this);
        connect(tutorialAction, &QAction::triggered, this, &MainWindow::showTutorialDialog);
        // Insert before About if present so the order is Tutorial / About.
        if (ui->actionAbout)
        {
            ui->menuHelp->insertAction(ui->actionAbout, tutorialAction);
        }
        else
        {
            ui->menuHelp->addAction(tutorialAction);
        }
    }
}

void MainWindow::onNewGame()
{
    ui->mineFieldWidget->newGame(m_currentDifficulty);
    m_isReplay = false;
    if (m_replayAction)
    {
        m_replayAction->setEnabled(false);
    }
    clearPauseState();
    resetTimerUi();
    setSmileyState(GameState::Ready);
    setWindowTitle(tr("QMineSweeper"));
    refitWindowToContents();
    Telemetry::addBreadcrumb(QStringLiteral("ui"), QStringLiteral("new game"));
}

void MainWindow::onReplaySameLayout()
{
    const bool replayed = ui->mineFieldWidget->newGameReplay();
    m_isReplay = replayed;
    if (m_replayAction)
    {
        // After a replay, the action stays enabled — the same layout is still
        // available. Only a fresh newGame() wipes it.
        m_replayAction->setEnabled(replayed);
    }
    clearPauseState();
    resetTimerUi();
    setSmileyState(GameState::Ready);
    setWindowTitle(tr("QMineSweeper"));
    refitWindowToContents();
    Telemetry::addBreadcrumb(QStringLiteral("ui"), replayed ? QStringLiteral("replay same layout") : QStringLiteral("replay fallback to new"));
}

void MainWindow::onDifficultyChanged(Difficulty diff)
{
    m_currentDifficulty = diff;
    m_isCustom = false;
    QSettings settings;
    settings.setValue("difficulty", difficultyName(diff));
    ui->mineFieldWidget->newGame(diff);
    m_isReplay = false;
    if (m_replayAction)
    {
        m_replayAction->setEnabled(false);
    }
    clearPauseState();
    resetTimerUi();
    setSmileyState(GameState::Ready);
    setWindowTitle(tr("QMineSweeper"));
    refitWindowToContents();
    Telemetry::addBreadcrumb(QStringLiteral("ui"), QStringLiteral("difficulty: ") + difficultyName(diff));
}

void MainWindow::onDifficultyCustom()
{
    Difficulty out;
    if (!showCustomDifficultyDialog(out))
    {
        // User cancelled — QActionGroup already moved the tick to Custom when
        // the action triggered, so flip it back to whatever difficulty is
        // actually active.
        recheckCurrentDifficultyAction();
        return;
    }

    m_currentDifficulty = out;
    m_isCustom = true;
    QSettings settings;
    settings.setValue("difficulty", QStringLiteral("Custom"));
    settings.setValue(QStringLiteral("custom_width"), out.width);
    settings.setValue(QStringLiteral("custom_height"), out.height);
    settings.setValue(QStringLiteral("custom_mines"), out.mineCount);
    ui->mineFieldWidget->newGame(out);
    m_isReplay = false;
    if (m_replayAction)
    {
        m_replayAction->setEnabled(false);
    }
    clearPauseState();
    resetTimerUi();
    setSmileyState(GameState::Ready);
    setWindowTitle(tr("QMineSweeper"));
    refitWindowToContents();
    Telemetry::addBreadcrumb(QStringLiteral("ui"), QStringLiteral("difficulty: Custom %1x%2/%3").arg(out.width).arg(out.height).arg(out.mineCount));
}

bool MainWindow::showCustomDifficultyDialog(Difficulty &out)
{
    QSettings s;
    const int initW = std::clamp(s.value(QStringLiteral("custom_width"), 30).toInt(), 9, 30);
    const int initH = std::clamp(s.value(QStringLiteral("custom_height"), 16).toInt(), 9, 24);
    const int initMaxM = std::max(10, initW * initH - 9);
    const int initM = std::clamp(s.value(QStringLiteral("custom_mines"), 99).toInt(), 10, initMaxM);

    QDialog dlg(this);
    dlg.setWindowTitle(tr("Custom difficulty"));

    auto *layout = new QVBoxLayout(&dlg);
    auto *form = new QFormLayout;

    auto *widthSpin = new QSpinBox(&dlg);
    widthSpin->setRange(9, 30);
    widthSpin->setValue(initW);
    form->addRow(tr("Width:"), widthSpin);

    auto *heightSpin = new QSpinBox(&dlg);
    heightSpin->setRange(9, 24);
    heightSpin->setValue(initH);
    form->addRow(tr("Height:"), heightSpin);

    auto *minesSpin = new QSpinBox(&dlg);
    // The 3×3 first-click safety zone needs up to 9 safe cells; keep the
    // ceiling at total - 9 so Custom boards can always guarantee a zero-start.
    const auto recomputeMinesRange = [widthSpin, heightSpin, minesSpin]()
    {
        const int total = widthSpin->value() * heightSpin->value();
        const int maxM = std::max(10, total - 9);
        const int current = minesSpin->value();
        minesSpin->setRange(10, maxM);
        minesSpin->setValue(std::clamp(current, 10, maxM));
    };
    minesSpin->setRange(10, initMaxM);
    minesSpin->setValue(initM);
    connect(widthSpin, qOverload<int>(&QSpinBox::valueChanged), &dlg, [recomputeMinesRange](int) { recomputeMinesRange(); });
    connect(heightSpin, qOverload<int>(&QSpinBox::valueChanged), &dlg, [recomputeMinesRange](int) { recomputeMinesRange(); });
    form->addRow(tr("Mines:"), minesSpin);

    layout->addLayout(form);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    layout->addWidget(buttons);

    if (dlg.exec() != QDialog::Accepted)
    {
        return false;
    }

    out.width = static_cast<std::uint32_t>(widthSpin->value());
    out.height = static_cast<std::uint32_t>(heightSpin->value());
    out.mineCount = static_cast<std::uint32_t>(minesSpin->value());
    return true;
}

void MainWindow::recheckCurrentDifficultyAction()
{
    const QString targetKey = m_isCustom ? QStringLiteral("Custom") : difficultyName(m_currentDifficulty);
    const QList<QAction *> diffActions = m_difficultyGroup ? m_difficultyGroup->actions() : QList<QAction *>{};
    for (QAction *action : diffActions)
    {
        if (action->data().toString() == targetKey)
        {
            action->setChecked(true);
            return;
        }
    }
}

void MainWindow::refitWindowToContents()
{
    // Unlock any previous setFixedSize constraint so adjustSize() can grow or
    // shrink the window freely.
    setMinimumSize(0, 0);
    setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    // MineField::buildGrid() calls setFixedSize() on itself, which posts a
    // LayoutRequest event to its parent (the central widget). We're still
    // inside the slot the user triggered, so that event hasn't been delivered
    // yet — meaning sizeHint() below would return the previous grid's size
    // and the window would stay sized to the previous difficulty. Activate
    // the outer layout synchronously so it picks up the new MineField size
    // BEFORE we sample sizeHint().
    if (QLayout *cl = centralWidget() ? centralWidget()->layout() : nullptr)
    {
        cl->activate();
    }
    adjustSize();
    setFixedSize(sizeHint());
}

void MainWindow::onGameStarted()
{
    m_gameTimer.start();
    m_pausedTotalMs = 0;
    m_pauseStartMs = 0;
    m_displayTimer->start();
    setSmileyState(GameState::Playing);
    setWindowTitle(tr("QMineSweeper — Playing"));
    // Once a mine layout exists, the user can replay it.
    if (m_replayAction)
    {
        m_replayAction->setEnabled(true);
    }
    if (m_pauseAction)
    {
        m_pauseAction->setEnabled(true);
    }
    Telemetry::recordEvent(QStringLiteral("game.started"), {
                                                               {QStringLiteral("difficulty"), difficultyName(m_currentDifficulty)},
                                                               {QStringLiteral("cols"), m_currentDifficulty.width},
                                                               {QStringLiteral("rows"), m_currentDifficulty.height},
                                                               {QStringLiteral("mines"), m_currentDifficulty.mineCount},
                                                               {QStringLiteral("replay"), m_isReplay ? QStringLiteral("true") : QStringLiteral("false")},
                                                           });
}

void MainWindow::onGameWon()
{
    m_displayTimer->stop();
    m_lastElapsedSeconds = elapsedSeconds();
    clearPauseState();
    updateTimerLabel();
    setSmileyState(GameState::Won);
    setWindowTitle(tr("QMineSweeper — You won!"));
    const QString diffName = difficultyName(m_currentDifficulty);
    // Replays don't update played/won counters or best-time — the layout was
    // already seen, so counting the win would let the user inflate stats by
    // memorising one board. Custom games are excluded for the same reason the
    // Stats dialog only lists the three standard presets — a lifetime best at
    // an arbitrary grid size is not comparable to the standards.
    const bool excludedFromStats = m_isReplay || m_isCustom;
    const bool noflagWin = !ui->mineFieldWidget->anyFlagPlaced();
    const int bv = ui->mineFieldWidget->boardValue();
    // Guard against div-by-zero on a sub-tick win (pathological setFixedLayout
    // case in tests; in real play the timer always advances at least 0.1s).
    const double bvRate = (m_lastElapsedSeconds > 0.05) ? (bv / m_lastElapsedSeconds) : 0.0;
    Stats::WinOutcome outcome{};
    if (!excludedFromStats)
    {
        // Threading bvRate into recordWin lets the persistence layer maintain
        // the per-difficulty `bestBvPerSecond` hall-of-fame; the returned
        // WinOutcome.newBestBvPerSecond drives the win-dialog
        // `⚡ New best 3BV/s!` flair, parallel to `🏆 New record!` /
        // `🌟 New best streak!`.
        outcome = Stats::recordWin(diffName, m_lastElapsedSeconds, QDate::currentDate(), bvRate);
    }
    const bool newRecord = outcome.newRecord;
    if (!excludedFromStats && noflagWin)
    {
        Stats::recordNoflagBest(diffName, m_lastElapsedSeconds);
    }
    const int clicks = ui->mineFieldWidget->userClicks();
    // Efficiency = 3BV / useful clicks · 100, rounded. Uncapped — chord-heavy
    // play legitimately yields >100 % and the speedrun community reports it.
    const int efficiency = (clicks > 0) ? static_cast<int>(std::lround(100.0 * bv / clicks)) : 0;
    Telemetry::recordEvent(QStringLiteral("game.won"), {
                                                           {QStringLiteral("difficulty"), diffName},
                                                           {QStringLiteral("duration_seconds"), QString::asprintf("%.1f", m_lastElapsedSeconds)},
                                                           {QStringLiteral("new_record"), newRecord ? QStringLiteral("true") : QStringLiteral("false")},
                                                           {QStringLiteral("replay"), m_isReplay ? QStringLiteral("true") : QStringLiteral("false")},
                                                           {QStringLiteral("noflag"), noflagWin ? QStringLiteral("true") : QStringLiteral("false")},
                                                           {QStringLiteral("bv"), QString::number(bv)},
                                                           {QStringLiteral("bv_per_second"), QString::asprintf("%.2f", bvRate)},
                                                           {QStringLiteral("clicks"), QString::number(clicks)},
                                                           {QStringLiteral("efficiency"), QString::number(efficiency)},
                                                           {QStringLiteral("streak"), QString::number(outcome.currentStreak)},
                                                           {QStringLiteral("new_best_streak"), outcome.newBestStreak ? QStringLiteral("true") : QStringLiteral("false")},
                                                           {QStringLiteral("new_best_bv_per_second"), outcome.newBestBvPerSecond ? QStringLiteral("true") : QStringLiteral("false")},
                                                       });
    // Average winning time on this difficulty — surfaced in the win dialog
    // once `winsAfter >= 3` (see `showEndDialog`). 0.0 is the "do not show"
    // sentinel: replays / customs (excludedFromStats) never call recordWin
    // so the outcome is the default-constructed WinOutcome with zeroed
    // wins/average; threshold-gating in the dialog handles the rest.
    showEndDialog(true, newRecord, noflagWin, bv, bvRate, clicks, efficiency, 0, outcome.currentStreak, outcome.newBestStreak, 0, 0, 0, 0.0, false, outcome.newBestBvPerSecond, 0, false,
                  (outcome.winsAfter >= 3) ? outcome.averageSecondsAfter : 0.0, QDate{}, 0u, (outcome.winsAfter >= 3) ? outcome.bestSecondsAfter : 0.0, 0.0, 0.0, 0u, outcome.priorBestSeconds);
}

void MainWindow::onGameLost(std::uint32_t /*row*/, std::uint32_t /*col*/)
{
    m_displayTimer->stop();
    m_lastElapsedSeconds = elapsedSeconds();
    clearPauseState();
    updateTimerLabel();
    setSmileyState(GameState::Lost);
    setWindowTitle(tr("QMineSweeper — Boom"));
    const QString diffName = difficultyName(m_currentDifficulty);
    const int safePercent = ui->mineFieldWidget->safePercentCleared();
    const int clicks = ui->mineFieldWidget->userClicks();
    const int flags = ui->mineFieldWidget->flagsPlaced();
    const int correctFlags = ui->mineFieldWidget->correctFlagsPlaced();
    const int bv = ui->mineFieldWidget->boardValue();
    const int qmarks = ui->mineFieldWidget->questionMarksPlaced();
    const int partialBv = ui->mineFieldWidget->partialBoardValue();
    // Rounded percentage of the user's placed flags that landed on actual
    // mines at the moment of explosion. 0 when no flags were placed (the
    // metric is undefined), which doubles as Stats::recordLoss's "skip the
    // best-flag-accuracy update" sentinel — a no-flag boom never sets the
    // record, mirroring the v1.32 loss-dialog gate that hides the
    // `Correct flags: %1 / %2` line when `flagsPlaced == 0`.
    const int flagAccuracyPercent = (flags > 0) ? static_cast<int>(std::lround(100.0 * correctFlags / flags)) : 0;
    // Read the full prior record *before* recordLoss runs. Three loss-dialog
    // lines depend on per-difficulty win history that recordLoss doesn't touch:
    // `Last win: %1` (lastWinDate), `Average: %1` (totalSecondsWon / won), and
    // its `(best %1)` companion (bestSeconds). Loading once and passing the
    // values keeps the read explicit at the call site and makes the "no prior
    // win → don't render" gates easy to follow. For custom games / replays we
    // still load — the lines reflect the per-difficulty record, not whether
    // this loss was counted; on Custom the load returns the default-constructed
    // record with `won == 0` and an invalid `lastWinDate` so all three lines
    // stay hidden.
    const Stats::Record priorRecord = Stats::load(diffName);
    const QDate priorLastWinDate = priorRecord.lastWinDate;
    // Loss-side mirror of the win-side `winsAfter >= 3` gate. The same
    // threshold reasoning applies: fewer than three wins reduces to "average is
    // the best time" (n=1) or "single data point of variation" (n=2), neither
    // informative. `totalSecondsWon > 0.0` defends the divisor against the
    // pathological all-sub-tick case (won=3 but every win was 0.0s).
    const double lossAverageSeconds = (priorRecord.won >= 3 && priorRecord.totalSecondsWon > 0.0) ? (priorRecord.totalSecondsWon / priorRecord.won) : 0.0;
    // Companion to the loss-side Average line — same gate as the win-side
    // `(best %2)` suffix: only render when the Average renders, since the
    // suffix without the anchor line would lose its referent. Same defensive
    // `> 0.0` guard preserved at the call site so a stray default-constructed
    // record can't render `(best 0.0)`.
    const double lossBestSeconds = (lossAverageSeconds > 0.0) ? priorRecord.bestSeconds : 0.0;
    Stats::LossOutcome lossOutcome{};
    if (!m_isReplay && !m_isCustom)
    {
        // Pass safePercent so Stats can update the per-difficulty
        // best-partial-clear hall-of-fame (surfaced in the Stats dialog only
        // when the user has never won this difficulty). Replays / custom
        // games are excluded for the same reason wins are: a memorised
        // board would let the user inflate the lifetime record. The
        // returned LossOutcome carries `newBestSafePercent` so the loss
        // dialog can flair on a fresh hall-of-fame entry, parallel to the
        // win-side `🏆 New record!` flair. flagAccuracyPercent threads the
        // rounded accuracy into the new lifetime hall-of-fame field; same
        // gate (>0) and same strict-greater-than semantics as safePercent.
        lossOutcome = Stats::recordLoss(diffName, safePercent, flagAccuracyPercent);
    }
    // Post-update bestSafePercent — the value the loss dialog's `(best %1%)`
    // companion to the "You cleared X% of the board." line renders. recordLoss
    // mutates the field iff `lossOutcome.newBestSafePercent` returns true, in
    // which case the new value is the clamped just-played safePercent;
    // otherwise it stays at the prior record. Replays / Custom skip recordLoss
    // so the field collapses to `priorRecord.bestSafePercent` — which on Custom
    // is 0 (the dialog gate hides the line), and on a standard-difficulty
    // replay is the lifetime record (informative anchor regardless of whether
    // this loss counted, mirroring the v1.42 `Average: %1` line behaviour).
    // safePercentCleared() returns [0, 100] by construction, but the clamp is
    // defensive against any future change to that contract.
    const std::uint32_t lossBestSafePercent = lossOutcome.newBestSafePercent ? static_cast<std::uint32_t>(safePercent > 100 ? 100 : (safePercent < 0 ? 0 : safePercent)) : priorRecord.bestSafePercent;
    // Same sub-tick guard as the win path — a fast loss with zero elapsed time
    // would otherwise divide by zero. 0.05s mirrors the win-side threshold.
    const double partialBvRate = (m_lastElapsedSeconds > 0.05) ? (partialBv / m_lastElapsedSeconds) : 0.0;
    Telemetry::recordEvent(QStringLiteral("game.lost"), {
                                                            {QStringLiteral("difficulty"), diffName},
                                                            {QStringLiteral("duration_seconds"), QString::asprintf("%.1f", m_lastElapsedSeconds)},
                                                            {QStringLiteral("replay"), m_isReplay ? QStringLiteral("true") : QStringLiteral("false")},
                                                            {QStringLiteral("clicks"), QString::number(clicks)},
                                                            {QStringLiteral("flags"), QString::number(flags)},
                                                            {QStringLiteral("bv"), QString::number(bv)},
                                                            {QStringLiteral("partial_bv"), QString::number(partialBv)},
                                                            {QStringLiteral("partial_bv_per_second"), QString::asprintf("%.2f", partialBvRate)},
                                                            {QStringLiteral("qmarks"), QString::number(qmarks)},
                                                            {QStringLiteral("correct_flags"), QString::number(correctFlags)},
                                                            {QStringLiteral("flag_accuracy_percent"), QString::number(flagAccuracyPercent)},
                                                            {QStringLiteral("new_best_safe_percent"), lossOutcome.newBestSafePercent ? QStringLiteral("true") : QStringLiteral("false")},
                                                            {QStringLiteral("new_best_flag_accuracy"), lossOutcome.newBestFlagAccuracyPercent ? QStringLiteral("true") : QStringLiteral("false")},
                                                        });
    showEndDialog(false, false, false, 0, 0.0, clicks, 0, flags, 0, false, bv, qmarks, partialBv, partialBvRate, lossOutcome.newBestSafePercent, false, correctFlags, lossOutcome.newBestFlagAccuracyPercent, 0.0, priorLastWinDate,
                  lossOutcome.priorStreak, 0.0, lossAverageSeconds, lossBestSeconds, lossBestSafePercent, 0.0);
}

void MainWindow::toggleTelemetry(bool enabled) { Telemetry::setEnabled(enabled, m_releaseId); }

void MainWindow::toggleQuestionMarks(bool enabled)
{
    MineButton::setQuestionMarksEnabled(enabled);
    QSettings settings;
    settings.setValue(QStringLiteral("settings/question_marks"), enabled);
    // Sweep live cells so no `?` is left stranded when the user disables mid-game.
    if (!enabled)
    {
        ui->mineFieldWidget->clearAllQuestionMarks();
    }
    Telemetry::addBreadcrumb(QStringLiteral("ui"), enabled ? QStringLiteral("question marks: on") : QStringLiteral("question marks: off"));
}

void MainWindow::toggleColorBlindPalette(bool enabled)
{
    MineButton::setColorBlindPaletteEnabled(enabled);
    QSettings settings;
    settings.setValue(QStringLiteral("settings/colorblind_palette"), enabled);
    // Repaint already-opened numbered cells so the toggle is visible mid-game,
    // not only on the next-opened cell. Mined cells and the unopened tiles
    // stay untouched — their stylesheets aren't a function of the palette.
    ui->mineFieldWidget->refreshAllNumberStyles();
    Telemetry::addBreadcrumb(QStringLiteral("ui"), enabled ? QStringLiteral("colorblind palette: on") : QStringLiteral("colorblind palette: off"));
}

void MainWindow::onLanguageChosen(const QString &code)
{
    // Empty string means "Auto (system)".
    if (code.isEmpty())
    {
        Language::clearUserOverride();
    }
    else
    {
        Language::setUserOverride(code);
    }

    QMessageBox box(this);
    box.setWindowTitle(tr("Language changed"));
    box.setIcon(QMessageBox::Information);
    box.setText(tr("Language changes take effect after restart."));
    box.setInformativeText(tr("Restart QMineSweeper now?"));
    QPushButton *restart = box.addButton(tr("Restart now"), QMessageBox::AcceptRole);
    box.addButton(tr("Later"), QMessageBox::RejectRole);
    box.setDefaultButton(restart);
    box.exec();
    if (box.clickedButton() == restart)
    {
        restartApp();
    }
}

void MainWindow::restartApp()
{
    QProcess::startDetached(QApplication::applicationFilePath(), QApplication::arguments());
    qApp->quit();
}

void MainWindow::maybeAskTelemetryConsent()
{
    if (!Telemetry::isCompiledIn() || Telemetry::hasAskedConsent())
    {
        return;
    }
    QMessageBox box(this);
    box.setWindowTitle(tr("Help improve QMineSweeper"));
    box.setIcon(QMessageBox::Question);
    box.setText(tr("Would you like to send anonymous crash reports and usage data?"));
    box.setInformativeText(tr("We collect: app crashes, game results (win/loss, duration, difficulty),"
                              " OS name, CPU architecture, Qt version, and an anonymous install ID."
                              "<br/><br/>"
                              "We do <b>not</b> collect: your name, email, IP address, file paths,"
                              " or any in-game actions."
                              "<br/><br/>"
                              "You can change this later in <b>Settings</b>."));
    box.setTextFormat(Qt::RichText);
    QPushButton *yes = box.addButton(tr("Yes, send"), QMessageBox::AcceptRole);
    box.addButton(tr("No thanks"), QMessageBox::RejectRole);
    box.setDefaultButton(yes);
    box.exec();
    const bool accepted = box.clickedButton() == yes;
    Telemetry::setHasAskedConsent(true);
    Telemetry::setEnabled(accepted, m_releaseId);
    if (m_telemetryAction)
    {
        m_telemetryAction->setChecked(accepted);
    }
}

void MainWindow::resetTimerUi()
{
    m_displayTimer->stop();
    m_lastElapsedSeconds = 0.0;
    ui->Time->setText(formatElapsedTime(0.0));
}

void MainWindow::setSmileyState(GameState state)
{
    m_smileyState = state;
    // A new game / game end always clears any lingering tension flag — the
    // mouse-release that matched the earlier press may never arrive if the
    // cell-freeze on win/loss intercepted the event.
    m_smileyPressing = false;
    applySmiley();
}

void MainWindow::setSmileyTension(bool pressing)
{
    m_smileyPressing = pressing;
    applySmiley();
}

void MainWindow::applySmiley()
{
    if (ui && ui->smileyButton)
    {
        ui->smileyButton->setText(smileyForTensionState(m_smileyState, m_smileyPressing));
    }
}

double MainWindow::elapsedSeconds() const noexcept
{
    if (!m_gameTimer.isValid())
    {
        return 0.0;
    }
    const qint64 raw = m_gameTimer.elapsed();
    qint64 paused = m_pausedTotalMs;
    if (m_paused)
    {
        // Active pause segment — subtract the time accumulated since the
        // current pause began so the on-screen timer freezes mid-pause.
        paused += raw - m_pauseStartMs;
    }
    return static_cast<double>(raw - paused) / 1000.0;
}

void MainWindow::onTogglePause()
{
    // Pause is meaningful only mid-play. The action is disabled outside
    // Playing already, but guard defensively in case the shortcut fires
    // before the menu state catches up.
    if (ui->mineFieldWidget->state() != GameState::Playing)
    {
        return;
    }

    if (!m_paused)
    {
        m_pauseStartMs = m_gameTimer.elapsed();
        m_paused = true;
        // Snapshot the current playing time so updateTimerLabel — which falls
        // back to m_lastElapsedSeconds when the display timer is inactive —
        // freezes the on-screen counter at the moment of pause.
        m_lastElapsedSeconds = elapsedSeconds();
        m_displayTimer->stop();
        ui->mineFieldWidget->setPaused(true);
        Telemetry::addBreadcrumb(QStringLiteral("ui"), QStringLiteral("pause"));
    }
    else
    {
        m_pausedTotalMs += m_gameTimer.elapsed() - m_pauseStartMs;
        m_pauseStartMs = 0;
        m_paused = false;
        ui->mineFieldWidget->setPaused(false);
        m_displayTimer->start();
        Telemetry::addBreadcrumb(QStringLiteral("ui"), QStringLiteral("resume"));
    }
    updatePauseAction();
    updateTimerLabel();
}

void MainWindow::clearPauseState()
{
    m_paused = false;
    m_pausedTotalMs = 0;
    m_pauseStartMs = 0;
    if (ui && ui->mineFieldWidget)
    {
        ui->mineFieldWidget->setPaused(false);
    }
    if (m_pauseAction)
    {
        // Re-enable only when a game is actually in progress; the next
        // gameStarted will flip this on.
        m_pauseAction->setEnabled(false);
    }
    updatePauseAction();
}

void MainWindow::updatePauseAction()
{
    if (!m_pauseAction)
    {
        return;
    }
    m_pauseAction->setText(m_paused ? tr("&Resume") : tr("&Pause"));
}

void MainWindow::updateTimerLabel()
{
    const double secs = m_displayTimer->isActive() ? elapsedSeconds() : m_lastElapsedSeconds;
    ui->Time->setText(formatElapsedTime(secs));
}

void MainWindow::showEndDialog(bool won, bool newRecord, bool noflagWin, int boardValue, double bvPerSecond, int userClicks, int efficiencyPct, int flagsPlaced, std::uint32_t currentStreak, bool newBestStreak, int lossBoardValue,
                               int lossQuestionMarks, int lossPartialBoardValue, double lossBvPerSecond, bool lossNewBestSafePercent, bool winNewBestBvPerSecond, int lossCorrectFlags, bool lossNewBestFlagAccuracy, double winAverageSeconds,
                               const QDate &lossLastWinDate, std::uint32_t lossPriorStreak, double winBestSeconds, double lossAverageSeconds, double lossBestSeconds, std::uint32_t lossBestSafePercent, double winPriorBestSeconds)
{
    QMessageBox box(this);
    box.setWindowTitle(won ? tr("You won!") : tr("Boom"));
    if (won)
    {
        QString text = tr("You cleared the field in %1.").arg(formatElapsedTime(m_lastElapsedSeconds));
        // Companion suffix anchoring the new-record run against the value being
        // beaten. Mirror of the v1.47 loss-side `(best %1%)` arrangement:
        // suffix lives on the same logical line as its anchor, separated by a
        // single space. Combined gate `newRecord && winPriorBestSeconds > 0.0`
        // keeps the line out of (a) non-record wins (where the v1.41
        // `Average: %1 (best %2)` line already supplies lifetime context, with
        // `(best …)` rendering the post-update best) and (b) first-ever wins
        // (where "previous record" has no referent — the pre-update best is
        // the 0.0 sentinel). Defensive `> 0.0` guard also catches the
        // pathological all-sub-tick prior-wins case where `bestSeconds` may
        // have stayed at 0.0 even with `won > 0`. Co-fires with the v1.45
        // `💎 Two new bests!` combo (which implies newRecord) — the
        // magnitude-of-improvement context applies regardless of which
        // prepended flair celebrates the record.
        if (newRecord && winPriorBestSeconds > 0.0)
        {
            text += QStringLiteral(" ") + tr("(prev %1)").arg(formatElapsedTime(winPriorBestSeconds));
        }
        // Lifetime average winning time on the current difficulty — surfaced
        // only when winsAfter >= 3 (the caller already encoded the gate by
        // passing 0.0 for fewer wins). Reuses formatElapsedTime so the
        // grammar matches the duration line above; no new format helpers.
        // 0.0 doubles as the "do not show" sentinel for replays / custom
        // games (which don't call Stats::recordWin) and for the first two
        // wins on each difficulty.
        if (winAverageSeconds > 0.0)
        {
            text += QStringLiteral("\n") + tr("Average: %1").arg(formatElapsedTime(winAverageSeconds));
            // Companion suffix anchoring the average against the player's
            // hall-of-fame best time on the same difficulty. Always shown
            // when the Average line shows: `winAverageSeconds > 0.0` implies
            // at least one counted non-sub-tick win, which (via the shared
            // `seconds > 0.0` gate in `Stats::recordWin`) implies
            // `winBestSeconds > 0.0`. Defensive `> 0.0` guard preserved
            // anyway so a stray default-constructed WinOutcome can't render
            // `(best 0.0)`.
            if (winBestSeconds > 0.0)
            {
                text += QStringLiteral(" ") + tr("(best %1)").arg(formatElapsedTime(winBestSeconds));
            }
        }
        // Speedrun efficiency footer — 3BV is the canonical Minesweeper board
        // value (minimum left-clicks to clear); 3BV/s is the per-second rate.
        // Always shown on wins regardless of replay/custom — it's a property
        // of the run itself, not a leaderboard claim.
        if (boardValue > 0)
        {
            text += QStringLiteral("\n") + tr("3BV: %1 · 3BV/s: %2").arg(boardValue).arg(QString::asprintf("%.2f", bvPerSecond));
        }
        // Click count and efficiency = 3BV / clicks · 100. Skipped when the
        // game was won without a single counted gesture (only reachable from
        // fixed-layout test setups), mirroring the bv == 0 guard above.
        if (userClicks > 0)
        {
            text += QStringLiteral("\n") + tr("Clicks: %1 · Efficiency: %2%").arg(userClicks).arg(efficiencyPct);
        }
        if (noflagWin)
        {
            text.prepend(tr("🏃 No-flag run!") + QStringLiteral("  "));
        }
        // Combo flair: when the same win sets BOTH a fresh per-difficulty
        // best clock-time AND a fresh per-difficulty best 3BV/s, swap both
        // individual flairs (`🏆 New record!` and `⚡ New best 3BV/s!`) for a
        // single celebratory line. Avoids the double-celebration stack of
        // 🏆 + ⚡ on a rare both-fire run and gives that moment its own beat.
        // Mirror of the v1.44 loss-side `🌟 Best loss yet!` arrangement, but
        // 💎 is picked over 🌟 because 🌟 is already the win-side `New best
        // streak` glyph (v1.16) — co-firing the combo and a streak-record on
        // one win would otherwise render two `🌟` prepends side-by-side.
        // Mutually exclusive with the individual 🏆 / ✨ / ⚡ flairs via the
        // outer-`if`-skip on the 🏆/✨ block and the `if/else if` shape on
        // the bottom 💎/⚡ pair; `✨ Beat your average!` is also skipped in
        // the combo case because beating the lifetime mean is implied by
        // setting a new best clock-time, so reading "💎 ✨" together would
        // re-celebrate the same axis. Streak slot is independent and may
        // still co-fire (e.g. `💎 Two new bests!  🌟 New best streak: N!`)
        // since a streak record is a different axis (counted run length,
        // not run quality).
        if (!(newRecord && winNewBestBvPerSecond))
        {
            if (newRecord)
            {
                text.prepend(tr("🏆 New record!") + QStringLiteral("  "));
            }
            // Softer lifetime-context flair when the just-finished run was strictly
            // faster than the player's lifetime mean on this difficulty but did NOT
            // set a new best. Mutually exclusive with `🏆 New record!` via `else if`
            // — a new record always implies beating the average (since `bestSeconds
            // <= mean` once n >= 1), so showing both would be redundant noise. Same
            // mutual-exclusion shape the streak slot uses for `🌟` vs `🔥`. Gates:
            //   • `winAverageSeconds > 0.0` — caller side already encodes the
            //     `winsAfter >= 3` threshold by passing 0.0 below it; replays /
            //     custom games never seed it (they don't call recordWin).
            //   • `m_lastElapsedSeconds > 0.0` — defends against a sub-tick win
            //     vacuously satisfying `0 < positive_mean`. Sub-tick wins are only
            //     reachable via setFixedLayout-driven tests, but the guard is cheap
            //     and pins honest behaviour.
            //   • `m_lastElapsedSeconds < winAverageSeconds` — strict-less; tying
            //     the average is not "beating" it.
            else if (winAverageSeconds > 0.0 && m_lastElapsedSeconds > 0.0 && m_lastElapsedSeconds < winAverageSeconds)
            {
                text.prepend(tr("✨ Beat your average!") + QStringLiteral("  "));
            }
        }
        // Streak flair — `newBestStreak` wins over the plain streak line so
        // the user never sees both. Streak >= 2 keeps the noise off when a
        // single win already feels celebratory enough.
        if (newBestStreak && currentStreak >= 2)
        {
            text.prepend(tr("🌟 New best streak: %1!").arg(currentStreak) + QStringLiteral("  "));
        }
        else if (currentStreak >= 2)
        {
            text.prepend(tr("🔥 Streak: %1").arg(currentStreak) + QStringLiteral("  "));
        }
        // 3BV/s record flair — fires on a strict-greater-than personal best
        // for this difficulty. Independent of `newRecord` (best clock time)
        // because best-time and best-3BV/s are independent records: a faster
        // win on a smaller board can set a new clock without touching 3BV/s,
        // and vice versa. Prepended last so it ends up leftmost — reading
        // left-to-right the player sees ⚡ first, then 🌟/🔥, then 🏆, then
        // 🏃, then the base "You cleared the field in …" line. The combo
        // arm above takes this slot when both records co-fire so 💎 ends up
        // leftmost in that case (and ⚡ is suppressed via the `else if`).
        if (newRecord && winNewBestBvPerSecond)
        {
            text.prepend(tr("💎 Two new bests!") + QStringLiteral("  "));
        }
        else if (winNewBestBvPerSecond)
        {
            text.prepend(tr("⚡ New best 3BV/s!") + QStringLiteral("  "));
        }
        box.setText(text);
        box.setIcon(QMessageBox::Information);
    }
    else
    {
        QString text = tr("You stepped on a mine.");
        text += QStringLiteral("\n") + tr("You survived for %1.").arg(formatElapsedTime(m_lastElapsedSeconds));
        text += QStringLiteral("\n") + tr("You cleared %1% of the board.").arg(ui->mineFieldWidget->safePercentCleared());
        // Companion suffix anchoring the just-played partial-clear against the
        // player's lifetime best on the same difficulty. Mirror of the v1.41
        // win-side `Average: %1 (best %2)` arrangement: the suffix lives on
        // the same logical line as its anchor, separated by a single space.
        // Gate is `lossBestSafePercent > 0` — hides the line on the very
        // first loss for a difficulty with no prior partial-clear record
        // (e.g. first-click boom on a fresh install) AND on Custom games
        // (where the per-difficulty record is always 0). On replays the
        // line still renders if the player has a standing record on the
        // standard difficulty, by design — the line reflects the per-
        // difficulty lifetime record, not whether *this* loss counted
        // (mirror of the v1.42 `Last win: %1` / `Average: %1` lines).
        // Distinct translation key from v1.41's `(best %1)` because the
        // percent suffix is part of the source string, allowing each locale
        // to render the percent variant with its own punctuation / digits.
        if (lossBestSafePercent > 0)
        {
            text += QStringLiteral(" ") + tr("(best %1%)").arg(lossBestSafePercent);
        }
        // Speedrun-canonical "cleared 3BV" line. X = how many of the board's
        // 3BV "clicks" the player effectively completed at the moment of
        // explosion (opened openings + opened isolated numbered cells); Y =
        // total board 3BV; Z = X / elapsed = the player's per-second clearing
        // pace at the moment of death. Subsumes (and replaces) the v1.25.0
        // static "Board 3BV: %1" line — strict superset of the same info.
        // Gated on lossBoardValue (= Y) > 0 because boardValue is unset before
        // the first click triggers placeMines — pathological loss paths (only
        // setFixedLayout-with-zero-mines or a pre-placement loss, neither
        // reachable in real play) skip the line.
        if (lossBoardValue > 0)
        {
            text += QStringLiteral("\n") + tr("Partial 3BV: %1 / %2 · 3BV/s: %3").arg(lossPartialBoardValue).arg(lossBoardValue).arg(QString::asprintf("%.2f", lossBvPerSecond));
        }
        // Click count mirrors the win-dialog's "Clicks: %1 · Efficiency: %2%"
        // line minus the efficiency suffix — efficiency = 3BV / clicks · 100
        // assumes a complete board, which a loss isn't. Gated by `userClicks
        // > 0` so the pathological setFixedLayout-with-zero-reveals path
        // (only reachable in tests) doesn't render a noisy "Clicks: 0".
        if (userClicks > 0)
        {
            text += QStringLiteral("\n") + tr("Clicks: %1").arg(userClicks);
        }
        // Flag-placement metric — completes the picture of player actions
        // before death (alongside Clicks). Gated on flagsPlaced > 0 so the
        // common "no flags" loss (especially fast booms) doesn't render a
        // noisy "Flags: 0" line. Reads m_flagCount at gameLost emission;
        // revealAllMines does not auto-flag, so the value is the user's true
        // count (mirror of the win-path which DOES auto-flag — that's why we
        // hide the line on wins regardless).
        if (flagsPlaced > 0)
        {
            text += QStringLiteral("\n") + tr("Flags placed: %1").arg(flagsPlaced);
            // Companion line giving flag-accuracy at the moment of explosion:
            // how many of the placed flags were on actual mines. Always rendered
            // alongside "Flags placed" (same gate) so the player who chose to
            // flag at all gets the matching accuracy readout. Form mirrors the
            // "Partial 3BV: X / Y" pair-line: numerator is correct flags,
            // denominator is total flags. correctFlags ≤ flagsPlaced is invariant
            // — the field can never count a flag that isn't placed. A perfect
            // flagger reads "Correct flags: 8 / 8" and a chaotic guess reads
            // "Correct flags: 0 / 8".
            text += QStringLiteral("\n") + tr("Correct flags: %1 / %2").arg(lossCorrectFlags).arg(flagsPlaced);
        }
        // Question-mark count — completes the right-click action recap alongside
        // Flags. Mirrors the flagsPlaced gate exactly: gated > 0 so a common no-`?`
        // loss doesn't render a noisy "Question marks: 0", and skipped on wins
        // because the win-path's flagAllMines does not touch question marks but
        // `?` only ever appears on Question-cycled cells (the question-marks toggle
        // can be off entirely). Reads MineField::questionMarksPlaced() at gameLost
        // emission; the loss path's revealAllMines does not clear m_marker, so the
        // value is the user's true count of `?`-marked cells.
        if (lossQuestionMarks > 0)
        {
            text += QStringLiteral("\n") + tr("Question marks: %1").arg(lossQuestionMarks);
        }
        // "💔 Streak ended at %1" — surfaced when this loss broke an active
        // winning streak of 2 or more on this difficulty. Mirrors the
        // win-side `🔥 Streak: %1` gate (currentStreak >= 2): a single-win
        // streak is not a streak worth mourning, and 0 means the previous
        // game was already a loss (no streak to break). Replays / custom
        // games never call `Stats::recordLoss`, so `lossPriorStreak` stays
        // at the default-constructed `LossOutcome{}.priorStreak == 0` and
        // the line stays hidden — by design, since those losses don't
        // actually break the standard-difficulty streak. Recap-line styling
        // (no `prepend`) places it in the loss narrative right before
        // "Last win: …", giving the player a two-beat closing arc:
        // momentum lost (Streak ended), but you've done this before
        // (Last win).
        if (lossPriorStreak >= 2)
        {
            text += QStringLiteral("\n") + tr("💔 Streak ended at %1").arg(lossPriorStreak);
        }
        // Most-recent-win timestamp for this difficulty — psychological nudge
        // anchoring the loss to the player's broader history ("you've done this
        // before, just not today"). Gated on `isValid()` so the line stays
        // hidden until the player has at least one counted win on this
        // difficulty: pre-1.37 plists (no `last_win_date` key) and Custom games
        // (no per-difficulty record) both load as invalid. Date is rendered via
        // QLocale::ShortFormat — same locale-aware format the Stats dialog has
        // used since v1.3 for `bestDate` / `bestNoflagDate` / etc., so the
        // grammar is consistent across the whole app. Independent of
        // replay/custom status: the line reflects the per-difficulty record,
        // not whether *this* loss was counted.
        if (lossLastWinDate.isValid())
        {
            text += QStringLiteral("\n") + tr("Last win: %1").arg(QLocale().toString(lossLastWinDate, QLocale::ShortFormat));
        }
        // Loss-side mirror of the v1.36 win-dialog `Average: %1` line and v1.41
        // `(best %2)` companion. Surfaces the lifetime mean winning time on the
        // current difficulty so the player has the same lifetime-context anchor
        // on losses as they have on wins. Same `>= 3 wins` threshold as the
        // win-side (encoded by the call site passing 0.0 below the gate); same
        // `tr("Average: %1")` and `tr("(best %1)")` keys as the win-side, so
        // every existing hand translation carries over with zero churn.
        // Rendered after `Last win:` so the closing arc reads "you've done this
        // before (Last win) → here's your typical pace (Average) → here's how
        // close that pace was to your best (best …)".
        if (lossAverageSeconds > 0.0)
        {
            text += QStringLiteral("\n") + tr("Average: %1").arg(formatElapsedTime(lossAverageSeconds));
            if (lossBestSeconds > 0.0)
            {
                text += QStringLiteral(" ") + tr("(best %1)").arg(formatElapsedTime(lossBestSeconds));
            }
        }
        // Combo flair: when the same loss earns a fresh best partial-clear
        // AND a fresh best flag accuracy, swap both individual flairs for a
        // single celebratory line. Avoids the triple-prepend stack of
        // 🚩 + 🎯 + base text and gives the rare both-fire moment its own
        // beat. Mutually exclusive with the individual 🎯 / 🚩 flairs via
        // the outer `else` — same shape as the win-side `🌟 New best
        // streak!` vs `🔥 Streak: %1` slot and the v1.43 `🏆 New record!`
        // vs `✨ Beat your average!` arrangement. 🌟 was picked over a
        // third loss-side glyph to mirror the win-side compound-celebration
        // convention (🌟 already reads as a "tier-up" beat there).
        if (lossNewBestSafePercent && lossNewBestFlagAccuracy)
        {
            text.prepend(tr("🌟 Best loss yet!") + QStringLiteral("  "));
        }
        else
        {
            // Mirror of the win-side `🏆 New record!` prepend: a fresh per-difficulty
            // partial-clear hall-of-fame entry deserves a celebratory flair on the
            // loss dialog too. `lossNewBestSafePercent` is true iff `Stats::recordLoss`
            // strictly beat the prior `bestSafePercent` for this difficulty (only
            // possible on standard, non-replay difficulties — same gate as the
            // recorded-loss path itself), so a first-click boom (0% cleared) and a
            // replay loss never flair.
            if (lossNewBestSafePercent)
            {
                text.prepend(tr("🎯 New best %!") + QStringLiteral("  "));
            }
            // Mirror of the win-side `⚡ New best 3BV/s!` flair: a fresh per-difficulty
            // best-flag-accuracy hall-of-fame entry deserves a celebratory flair on the
            // loss dialog parallel to the v1.29 🎯 safe-percent flair. Independent of
            // `lossNewBestSafePercent` because the two records track different axes
            // (board coverage vs. flag-placement accuracy) and can co-fire on a single
            // loss; prepended last so 🚩 ends up leftmost when both fire — matches
            // the win-side convention of putting the newer flair leftmost. 🚩 was
            // chosen to map to the in-game flag mechanic and to stay visually
            // distinct from the existing 🎯. Gate is `lossOutcome.newBestFlagAccuracyPercent`,
            // which is false on replays / custom games (recordLoss is skipped) and on
            // any loss where flagsPlaced == 0 (recordLoss's no-flag sentinel).
            if (lossNewBestFlagAccuracy)
            {
                text.prepend(tr("🚩 New best flag accuracy!") + QStringLiteral("  "));
            }
        }
        box.setText(text);
        box.setIcon(QMessageBox::Warning);
    }
    QPushButton *newGameBtn = box.addButton(tr("New Game"), QMessageBox::AcceptRole);
    box.addButton(tr("Close"), QMessageBox::RejectRole);
    box.setDefaultButton(newGameBtn);
    box.exec();
    if (box.clickedButton() == newGameBtn)
    {
        onNewGame();
    }
}

void MainWindow::showTutorialDialog()
{
    TutorialDialog dlg(this);
    connect(&dlg, &TutorialDialog::completed,
            []
            {
                Tutorial::markCompleted();
                Telemetry::recordEvent(QStringLiteral("tutorial.completed"), {});
            });
    connect(&dlg, &TutorialDialog::skipped,
            []
            {
                // Mark completed on skip too — the whole point is not to re-prompt
                // on every launch once the user has declined once.
                Tutorial::markCompleted();
                Telemetry::recordEvent(QStringLiteral("tutorial.skipped"), {});
            });
    dlg.exec();
}

void MainWindow::showStatsDialog()
{
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Statistics"));

    auto *layout = new QVBoxLayout(&dlg);

    auto *table = new QTableWidget(4, 12, &dlg);
    table->setHorizontalHeaderLabels(
        {tr("Difficulty"), tr("Played"), tr("Won"), tr("Best time"), tr("Average"), tr("Best (no flag)"), tr("Streak"), tr("Best 3BV/s"), tr("Best partial"), tr("Best flag accuracy"), tr("Last win"), tr("Last loss")});
    table->verticalHeader()->setVisible(false);
    table->horizontalHeader()->setStretchLastSection(true);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setFocusPolicy(Qt::NoFocus);

    struct Row
    {
        const char *label;
        const char *key;
    };
    const Row rows[] = {
        {QT_TR_NOOP("Beginner"), "Beginner"},
        {QT_TR_NOOP("Intermediate"), "Intermediate"},
        {QT_TR_NOOP("Expert"), "Expert"},
    };
    const auto formatBest = [](double seconds, const QDate &date)
    {
        if (seconds <= 0.0)
        {
            return QStringLiteral("—");
        }
        // Duration-aware clock format matches the live timer label.
        // The "s" unit is dropped because the column header carries it
        // and "s" reads wrong on M:SS.S / H:MM:SS.S values.
        QString s = formatElapsedTime(seconds);
        if (date.isValid())
        {
            // Locale-formatted date in parentheses, e.g. "1:30.5 (23.04.2026)".
            // Inline (vs. a separate column) keeps the row narrow and avoids
            // introducing another translatable column header per stat.
            s += QStringLiteral("  (") + QLocale().toString(date, QLocale::ShortFormat) + QStringLiteral(")");
        }
        return s;
    };
    const auto formatStreak = [](std::uint32_t cur, std::uint32_t best, const QDate &bestDate)
    {
        if (cur == 0 && best == 0)
        {
            return QStringLiteral("—");
        }
        QString s = QStringLiteral("%1 / %2").arg(cur).arg(best);
        if (bestDate.isValid())
        {
            s += QStringLiteral("  (") + QLocale().toString(bestDate, QLocale::ShortFormat) + QStringLiteral(")");
        }
        return s;
    };
    // Last-win cell: locale-formatted date of the player's most-recent counted
    // win for this difficulty. Mirrors the v1.37.0 loss-dialog "Last win: %1"
    // line so a player who reads the loss dialog and then opens Stats sees the
    // same anchor across all three difficulties at once. Em-dash when the
    // player has never won this difficulty (or pre-1.37 plist with no
    // last_win_date key — clean-slate seeding by design).
    const auto formatLastWin = [](const QDate &date)
    {
        if (!date.isValid())
        {
            return QStringLiteral("—");
        }
        return QLocale().toString(date, QLocale::ShortFormat);
    };
    // Last-loss cell: locale-formatted date of the player's most-recent counted
    // loss for this difficulty. Mirror of `formatLastWin` on the loss axis;
    // surfaces the v1.49 `lastLossDate` field stamped unconditionally on every
    // counted recordLoss. Em-dash when the player has never lost this
    // difficulty since 1.49 (or pre-1.49 plist with no last_loss_date key —
    // clean-slate seeding by design).
    const auto formatLastLoss = [](const QDate &date)
    {
        if (!date.isValid())
        {
            return QStringLiteral("—");
        }
        return QLocale().toString(date, QLocale::ShortFormat);
    };
    std::uint64_t totalPlayed = 0;
    std::uint64_t totalWon = 0;
    for (int i = 0; i < 3; ++i)
    {
        const Stats::Record rec = Stats::load(QString::fromLatin1(rows[i].key));
        const QString winRate = rec.played > 0 ? QStringLiteral(" (%1%)").arg(100 * rec.won / rec.played) : QString{};
        table->setItem(i, 0, new QTableWidgetItem(tr(rows[i].label)));
        table->setItem(i, 1, new QTableWidgetItem(QString::number(rec.played)));
        table->setItem(i, 2, new QTableWidgetItem(QString::number(rec.won) + winRate));
        table->setItem(i, 3, new QTableWidgetItem(formatBest(rec.bestSeconds, rec.bestDate)));
        table->setItem(i, 4, new QTableWidgetItem(formatAverageCell(rec.totalSecondsWon, rec.won)));
        table->setItem(i, 5, new QTableWidgetItem(formatBest(rec.bestNoflagSeconds, rec.bestNoflagDate)));
        table->setItem(i, 6, new QTableWidgetItem(formatStreak(rec.currentStreak, rec.bestStreak, rec.bestStreakDate)));
        table->setItem(i, 7, new QTableWidgetItem(formatBvPerSecondCell(rec.bestBvPerSecond, rec.bestBvPerSecondDate)));
        // Best partial: per-difficulty highest board-coverage percentage ever
        // reached on a *loss* (v1.29 `bestSafePercent`). Pre-1.46 this lived
        // as a fallback inside the Best-time cell that gated off as soon as
        // any win was recorded — surfaced now in its own column so the
        // partial-clear hall of fame stays visible regardless of win count,
        // mirroring the dedicated Best-flag-accuracy column added in v1.33.
        table->setItem(i, 8, new QTableWidgetItem(formatSafePercentCell(static_cast<int>(rec.bestSafePercent), rec.bestSafePercentDate)));
        table->setItem(i, 9, new QTableWidgetItem(formatFlagAccuracyCell(static_cast<int>(rec.bestFlagAccuracyPercent), rec.bestFlagAccuracyDate)));
        table->setItem(i, 10, new QTableWidgetItem(formatLastWin(rec.lastWinDate)));
        table->setItem(i, 11, new QTableWidgetItem(formatLastLoss(rec.lastLossDate)));
        totalPlayed += rec.played;
        totalWon += rec.won;
    }
    // "Total" row aggregates Played and Won across all three difficulties.
    // Best-time / average / best-streak / best-3BV/s / best-partial /
    // best-flag-accuracy / last-win / last-loss columns collapse to em-dash
    // because per-difficulty bests don't sum or average meaningfully across
    // difficulties of different sizes (the global lifetime mean
    // `sum(totalSecondsWon)/sum(won)` is mathematically defined but mostly
    // reflects whichever difficulty the player plays most — noise, not
    // signal). And "last win across any difficulty" would just shadow
    // whichever per-row date is most recent — duplicate signal, not new
    // information. Same applies to "last loss across any difficulty".
    const QString totalWinRate = totalPlayed > 0 ? QStringLiteral(" (%1%)").arg(100 * totalWon / totalPlayed) : QString{};
    QFont totalFont = table->font();
    totalFont.setBold(true);
    const auto makeTotalItem = [&totalFont](const QString &text)
    {
        auto *item = new QTableWidgetItem(text);
        item->setFont(totalFont);
        return item;
    };
    table->setItem(3, 0, makeTotalItem(tr("Total")));
    table->setItem(3, 1, makeTotalItem(QString::number(totalPlayed)));
    table->setItem(3, 2, makeTotalItem(QString::number(totalWon) + totalWinRate));
    table->setItem(3, 3, makeTotalItem(QStringLiteral("—")));
    table->setItem(3, 4, makeTotalItem(QStringLiteral("—")));
    table->setItem(3, 5, makeTotalItem(QStringLiteral("—")));
    table->setItem(3, 6, makeTotalItem(QStringLiteral("—")));
    table->setItem(3, 7, makeTotalItem(QStringLiteral("—")));
    table->setItem(3, 8, makeTotalItem(QStringLiteral("—")));
    table->setItem(3, 9, makeTotalItem(QStringLiteral("—")));
    table->setItem(3, 10, makeTotalItem(QStringLiteral("—")));
    table->setItem(3, 11, makeTotalItem(QStringLiteral("—")));
    table->resizeColumnsToContents();

    layout->addWidget(table);

    auto *buttons = new QDialogButtonBox(&dlg);
    auto *resetBtn = buttons->addButton(tr("Reset all"), QDialogButtonBox::DestructiveRole);
    buttons->addButton(QDialogButtonBox::Close);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    connect(resetBtn, &QPushButton::clicked,
            [this, &dlg]
            {
                const auto answer = QMessageBox::question(this, tr("Reset statistics?"), tr("Permanently erase all played / won / best-time records?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
                if (answer == QMessageBox::Yes)
                {
                    Stats::resetAll();
                    dlg.accept();
                }
            });
    layout->addWidget(buttons);

    dlg.exec();
}

void MainWindow::showAboutDialog()
{
    const QString body = tr("<h3>QMineSweeper %1</h3>"
                            "<p>A Qt6-based Minesweeper game.</p>"
                            "<p>Left-click to reveal, right-click to flag,"
                            " middle-click on a satisfied number to chord.</p>"
                            "<p><b>Developed by AI</b> — every line of code, test, translation, and release note in this project was written by an AI assistant collaborating with the maintainer.</p>"
                            "<p>© Mavrikant</p>")
                             .arg(QString::fromUtf8(QMS_VERSION));
    const QString buildInfo = tr("<p><small>Built with Qt %1 on %2</small></p>").arg(QString::fromUtf8(QT_VERSION_STR), QString::fromUtf8(__DATE__ " " __TIME__));
    QMessageBox::about(this, tr("About QMineSweeper"), body + buildInfo);
}
