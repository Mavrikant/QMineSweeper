#ifndef MINEFIELD_H
#define MINEFIELD_H

#include "minebutton.h"

#include <QGridLayout>
#include <QLabel>
#include <QPointer>
#include <QWidget>

#include <cstdint>
#include <utility>
#include <vector>

enum class GameState
{
    Ready,
    Playing,
    Won,
    Lost
};

struct Difficulty
{
    std::uint32_t width{9};
    std::uint32_t height{9};
    std::uint32_t mineCount{10};
};

class MineField : public QWidget
{
    Q_OBJECT
  public:
    explicit MineField(QWidget *parent = nullptr);

    static constexpr Difficulty Beginner{9, 9, 10};
    static constexpr Difficulty Intermediate{16, 16, 40};
    static constexpr Difficulty Expert{30, 16, 99};

    void newGame();
    void newGame(Difficulty diff);

    [[nodiscard]] GameState state() const noexcept;
    [[nodiscard]] std::uint32_t cols() const noexcept;
    [[nodiscard]] std::uint32_t rows() const noexcept;
    [[nodiscard]] std::uint32_t mineCount() const noexcept;
    [[nodiscard]] int remainingMines() const noexcept;
    [[nodiscard]] const Difficulty &difficulty() const noexcept;

    void setMineCountLabel(QLabel *label);

    // Test helper: deterministic layout with explicit mine positions.
    void setFixedLayout(std::uint32_t width, std::uint32_t height, const std::vector<std::pair<std::uint32_t, std::uint32_t>> &minePositions);

    // Test helper: direct access for fixture-based tests.
    [[nodiscard]] MineButton *cellAt(std::uint32_t row, std::uint32_t col) const;

  signals:
    void gameStarted();
    void gameWon();
    void gameLost(std::uint32_t row, std::uint32_t col);
    void mineCountChanged(int remaining);

  private slots:
    void onCellPressed(std::uint32_t row, std::uint32_t col);
    void onCellOpened(std::uint32_t row, std::uint32_t col);
    void onMineExploded(std::uint32_t row, std::uint32_t col);
    void onFlagToggled(std::uint32_t row, std::uint32_t col, bool flagged);
    void onChordRequested(std::uint32_t row, std::uint32_t col);
    void onCheckNeighbours(std::uint32_t row, std::uint32_t col);

  private:
    void buildGrid();
    void clearGrid();
    void wireButton(MineButton *button);
    void fillMines(std::uint32_t safeRow, std::uint32_t safeCol);
    void fillNumbers();
    void revealAllMines();
    void flagAllMines();
    void freezeAllCells();
    void updateMineCountLabel();
    void checkWin();

    QGridLayout *m_grid{nullptr};
    std::vector<std::vector<MineButton *>> m_buttons;
    QPointer<QLabel> m_mineCountLabel;

    Difficulty m_difficulty{Beginner};
    GameState m_state{GameState::Ready};
    std::uint32_t m_openedSafeCount{0};
    int m_flagCount{0};
    bool m_minesPlaced{false};
};

#endif // MINEFIELD_H
