#include "mainwindow.h"

#include "./ui_mainwindow.h"
#include "language.h"
#include "stats.h"
#include "telemetry.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QKeySequence>
#include <QLabel>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QSettings>
#include <QTableWidget>
#include <QVBoxLayout>

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

    m_displayTimer = new QTimer(this);
    m_displayTimer->setInterval(50);
    connect(m_displayTimer, &QTimer::timeout, this, &MainWindow::updateTimerLabel);

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
    m_currentDifficulty = startDifficulty;

    // Tick the checkable difficulty action that matches the restored preset.
    const QList<QAction *> diffActions = m_difficultyGroup->actions();
    for (QAction *action : diffActions)
    {
        if (action->data().toString() == difficultyName(startDifficulty))
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

    resetTimerUi();
    setWindowTitle(tr("QMineSweeper"));

    centralWidget()->layout()->setContentsMargins(0, 0, 0, 0);
    adjustSize();
    setFixedSize(sizeHint());

    maybeAskTelemetryConsent();
}

MainWindow::~MainWindow() = default;

void MainWindow::buildMenus()
{
    auto *newAction = new QAction(tr("&New"), this);
    newAction->setShortcut(QKeySequence(QKeySequence::New));
    connect(newAction, &QAction::triggered, this, &MainWindow::onNewGame);
    ui->menuGame->addAction(newAction);

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
}

void MainWindow::onNewGame()
{
    ui->mineFieldWidget->newGame(m_currentDifficulty);
    resetTimerUi();
    setWindowTitle(tr("QMineSweeper"));
    setMinimumSize(0, 0);
    setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    adjustSize();
    setFixedSize(sizeHint());
    Telemetry::addBreadcrumb(QStringLiteral("ui"), QStringLiteral("new game"));
}

void MainWindow::onDifficultyChanged(Difficulty diff)
{
    m_currentDifficulty = diff;
    QSettings settings;
    settings.setValue("difficulty", difficultyName(diff));
    ui->mineFieldWidget->newGame(diff);
    resetTimerUi();
    setWindowTitle(tr("QMineSweeper"));
    setMinimumSize(0, 0);
    setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    adjustSize();
    setFixedSize(sizeHint());
    Telemetry::addBreadcrumb(QStringLiteral("ui"), QStringLiteral("difficulty: ") + difficultyName(diff));
}

void MainWindow::onGameStarted()
{
    m_gameTimer.start();
    m_displayTimer->start();
    setWindowTitle(tr("QMineSweeper — Playing"));
    Telemetry::recordEvent(QStringLiteral("game.started"), {
                                                               {QStringLiteral("difficulty"), difficultyName(m_currentDifficulty)},
                                                               {QStringLiteral("cols"), m_currentDifficulty.width},
                                                               {QStringLiteral("rows"), m_currentDifficulty.height},
                                                               {QStringLiteral("mines"), m_currentDifficulty.mineCount},
                                                           });
}

void MainWindow::onGameWon()
{
    m_displayTimer->stop();
    m_lastElapsedSeconds = elapsedSeconds();
    updateTimerLabel();
    setWindowTitle(tr("QMineSweeper — You won!"));
    const QString diffName = difficultyName(m_currentDifficulty);
    const bool newRecord = Stats::recordWin(diffName, m_lastElapsedSeconds);
    Telemetry::recordEvent(QStringLiteral("game.won"), {
                                                           {QStringLiteral("difficulty"), diffName},
                                                           {QStringLiteral("duration_seconds"), QString::asprintf("%.1f", m_lastElapsedSeconds)},
                                                           {QStringLiteral("new_record"), newRecord ? QStringLiteral("true") : QStringLiteral("false")},
                                                       });
    showEndDialog(true, newRecord);
}

void MainWindow::onGameLost(std::uint32_t /*row*/, std::uint32_t /*col*/)
{
    m_displayTimer->stop();
    m_lastElapsedSeconds = elapsedSeconds();
    updateTimerLabel();
    setWindowTitle(tr("QMineSweeper — Boom"));
    const QString diffName = difficultyName(m_currentDifficulty);
    Stats::recordLoss(diffName);
    Telemetry::recordEvent(QStringLiteral("game.lost"), {
                                                            {QStringLiteral("difficulty"), diffName},
                                                            {QStringLiteral("duration_seconds"), QString::asprintf("%.1f", m_lastElapsedSeconds)},
                                                        });
    showEndDialog(false, false);
}

void MainWindow::toggleTelemetry(bool enabled) { Telemetry::setEnabled(enabled, m_releaseId); }

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
        const QString best = rec.bestSeconds > 0.0 ? QString::asprintf("%.1f s", rec.bestSeconds) : QStringLiteral("—");
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
    QMessageBox::about(this, tr("About QMineSweeper"),
                       tr("<h3>QMineSweeper %1</h3>"
                          "<p>A Qt6-based Minesweeper game.</p>"
                          "<p>Left-click to reveal, right-click to flag,"
                          " middle-click on a satisfied number to chord.</p>"
                          "<p>© Mavrikant</p>")
                           .arg(QString::fromUtf8(QMS_VERSION)));
}
