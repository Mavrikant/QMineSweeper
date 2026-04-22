#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "minefield.h"

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
    void onDifficultyChanged(Difficulty diff);
    void onGameStarted();
    void onGameWon();
    void onGameLost(std::uint32_t row, std::uint32_t col);
    void showAboutDialog();

  private:
    void buildMenus();
    void resetTimerUi();
    void updateTimerLabel();
    void showEndDialog(bool won);
    [[nodiscard]] double elapsedSeconds() const noexcept;

    std::unique_ptr<Ui::MainWindow> ui;
    QElapsedTimer m_gameTimer;
    QTimer *m_displayTimer{nullptr};
    QActionGroup *m_difficultyGroup{nullptr};
    Difficulty m_currentDifficulty{MineField::Beginner};
    double m_lastElapsedSeconds{0.0};
};

#endif // MAINWINDOW_H
