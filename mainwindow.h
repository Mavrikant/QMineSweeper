#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "minefield.h"
#include "smiley.h"

#include <QDate>
#include <QElapsedTimer>
#include <QMainWindow>
#include <QTimer>

#include <memory>

QT_BEGIN_NAMESPACE
class QActionGroup;
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

  private slots:
    void onNewGame();
    void onReplaySameLayout();
    void onDifficultyChanged(Difficulty diff);
    void onDifficultyCustom();
    void onGameStarted();
    void onGameWon();
    void onGameLost(std::uint32_t row, std::uint32_t col);
    void onTogglePause();
    void showAboutDialog();
    void showStatsDialog();
    void showTutorialDialog();
    void toggleTelemetry(bool enabled);
    void onLanguageChosen(const QString &code);
    void toggleQuestionMarks(bool enabled);
    void toggleColorBlindPalette(bool enabled);

  private:
    void buildMenus();
    void resetTimerUi();
    void updateTimerLabel();
    void setSmileyState(GameState state);
    void setSmileyTension(bool pressing);
    void applySmiley();
    void showEndDialog(bool won, bool newRecord, bool noflagWin, int boardValue, double bvPerSecond, int userClicks, int efficiencyPct, int flagsPlaced, std::uint32_t currentStreak, bool newBestStreak, int lossBoardValue,
                       int lossQuestionMarks, int lossPartialBoardValue, double lossBvPerSecond, bool lossNewBestSafePercent, bool winNewBestBvPerSecond, int lossCorrectFlags, bool lossNewBestFlagAccuracy, double winAverageSeconds,
                       const QDate &lossLastWinDate, std::uint32_t lossPriorStreak, double winBestSeconds, double lossAverageSeconds, double lossBestSeconds, std::uint32_t lossBestSafePercent);
    void maybeAskTelemetryConsent();
    void restartApp();
    void refitWindowToContents();
    void recheckCurrentDifficultyAction();
    void clearPauseState();
    void updatePauseAction();
    [[nodiscard]] bool showCustomDifficultyDialog(Difficulty &out);
    [[nodiscard]] double elapsedSeconds() const noexcept;

    std::unique_ptr<Ui::MainWindow> ui;
    QElapsedTimer m_gameTimer;
    QTimer *m_displayTimer{nullptr};
    QActionGroup *m_difficultyGroup{nullptr};
    QActionGroup *m_languageGroup{nullptr};
    QAction *m_telemetryAction{nullptr};
    QAction *m_questionMarksAction{nullptr};
    QAction *m_colorBlindPaletteAction{nullptr};
    QAction *m_replayAction{nullptr};
    QAction *m_customDifficultyAction{nullptr};
    QAction *m_pauseAction{nullptr};
    Difficulty m_currentDifficulty{MineField::Beginner};
    double m_lastElapsedSeconds{0.0};
    bool m_isReplay{false};
    bool m_isCustom{false};
    bool m_paused{false};
    qint64 m_pausedTotalMs{0};
    qint64 m_pauseStartMs{0};
    GameState m_smileyState{GameState::Ready};
    bool m_smileyPressing{false};
    QString m_releaseId;
};

#endif // MAINWINDOW_H
