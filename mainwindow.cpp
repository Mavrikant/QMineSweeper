#include "mainwindow.h"

#include "./ui_mainwindow.h"
#include "language.h"
#include "stats.h"
#include "telemetry.h"
#include "tutorial.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
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
    m_displayTimer->start();
    setSmileyState(GameState::Playing);
    setWindowTitle(tr("QMineSweeper — Playing"));
    // Once a mine layout exists, the user can replay it.
    if (m_replayAction)
    {
        m_replayAction->setEnabled(true);
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
    const bool newRecord = !excludedFromStats && Stats::recordWin(diffName, m_lastElapsedSeconds);
    Telemetry::recordEvent(QStringLiteral("game.won"), {
                                                           {QStringLiteral("difficulty"), diffName},
                                                           {QStringLiteral("duration_seconds"), QString::asprintf("%.1f", m_lastElapsedSeconds)},
                                                           {QStringLiteral("new_record"), newRecord ? QStringLiteral("true") : QStringLiteral("false")},
                                                           {QStringLiteral("replay"), m_isReplay ? QStringLiteral("true") : QStringLiteral("false")},
                                                       });
    showEndDialog(true, newRecord);
}

void MainWindow::onGameLost(std::uint32_t /*row*/, std::uint32_t /*col*/)
{
    m_displayTimer->stop();
    m_lastElapsedSeconds = elapsedSeconds();
    updateTimerLabel();
    setSmileyState(GameState::Lost);
    setWindowTitle(tr("QMineSweeper — Boom"));
    const QString diffName = difficultyName(m_currentDifficulty);
    if (!m_isReplay && !m_isCustom)
    {
        Stats::recordLoss(diffName);
    }
    Telemetry::recordEvent(QStringLiteral("game.lost"), {
                                                            {QStringLiteral("difficulty"), diffName},
                                                            {QStringLiteral("duration_seconds"), QString::asprintf("%.1f", m_lastElapsedSeconds)},
                                                            {QStringLiteral("replay"), m_isReplay ? QStringLiteral("true") : QStringLiteral("false")},
                                                        });
    showEndDialog(false, false);
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
    ui->Time->setText(QStringLiteral("000.0"));
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

double MainWindow::elapsedSeconds() const noexcept { return static_cast<double>(m_gameTimer.elapsed()) / 1000.0; }

void MainWindow::updateTimerLabel()
{
    const double secs = m_displayTimer->isActive() ? elapsedSeconds() : m_lastElapsedSeconds;
    ui->Time->setText(QString::asprintf("%05.1f", secs));
}

void MainWindow::showEndDialog(bool won, bool newRecord)
{
    QMessageBox box(this);
    box.setWindowTitle(won ? tr("You won!") : tr("Boom"));
    if (won)
    {
        QString text = tr("You cleared the field in %1 seconds.").arg(QString::asprintf("%.1f", m_lastElapsedSeconds));
        if (newRecord)
        {
            text.prepend(tr("🏆 New record!") + QStringLiteral("  "));
        }
        box.setText(text);
        box.setIcon(QMessageBox::Information);
    }
    else
    {
        box.setText(tr("You stepped on a mine."));
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

    auto *table = new QTableWidget(3, 4, &dlg);
    table->setHorizontalHeaderLabels({tr("Difficulty"), tr("Played"), tr("Won"), tr("Best time")});
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
    for (int i = 0; i < 3; ++i)
    {
        const Stats::Record rec = Stats::load(QString::fromLatin1(rows[i].key));
        QString best;
        if (rec.bestSeconds > 0.0)
        {
            best = QString::asprintf("%.1f s", rec.bestSeconds);
            if (rec.bestDate.isValid())
            {
                // Locale-formatted date in parentheses, e.g. "15.5 s (23.04.2026)".
                // Inline (vs. a separate column) keeps the dialog narrow and avoids
                // introducing a new translatable column header.
                best += QStringLiteral("  (") + QLocale().toString(rec.bestDate, QLocale::ShortFormat) + QStringLiteral(")");
            }
        }
        else
        {
            best = QStringLiteral("—");
        }
        const QString winRate = rec.played > 0 ? QStringLiteral(" (%1%)").arg(100 * rec.won / rec.played) : QString{};
        table->setItem(i, 0, new QTableWidgetItem(tr(rows[i].label)));
        table->setItem(i, 1, new QTableWidgetItem(QString::number(rec.played)));
        table->setItem(i, 2, new QTableWidgetItem(QString::number(rec.won) + winRate));
        table->setItem(i, 3, new QTableWidgetItem(best));
    }
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
                            "<p>© Mavrikant</p>")
                             .arg(QString::fromUtf8(QMS_VERSION));
    const QString buildInfo = tr("<p><small>Built with Qt %1 on %2</small></p>").arg(QString::fromUtf8(QT_VERSION_STR), QString::fromUtf8(__DATE__ " " __TIME__));
    QMessageBox::about(this, tr("About QMineSweeper"), body + buildInfo);
}
