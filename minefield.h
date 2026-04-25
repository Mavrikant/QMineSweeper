#ifndef MINEFIELD_H
#define MINEFIELD_H

#include "minebutton.h"

#include <QFrame>
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

    // Start a fresh game on the most recently played mine layout. Returns true
    // if a prior layout existed and was reused; returns false and falls back
    // to a normal newGame() if no game has been started yet.
    bool newGameReplay();

    // True once a mine layout has been generated (i.e. the user has clicked
    // their first cell on the current difficulty).
    [[nodiscard]] bool canReplay() const noexcept;

    [[nodiscard]] GameState state() const noexcept;
    [[nodiscard]] std::uint32_t cols() const noexcept;
    [[nodiscard]] std::uint32_t rows() const noexcept;
    [[nodiscard]] std::uint32_t mineCount() const noexcept;
    [[nodiscard]] int remainingMines() const noexcept;
    [[nodiscard]] const Difficulty &difficulty() const noexcept;

    // Pause/resume — orthogonal to the GameState enum. While paused, the
    // eventFilter swallows all mouse + keyboard input on every cell, and
    // a translucent overlay covers the grid. The state machine is left
    // untouched (a paused game is still GameState::Playing) so win-check
    // and freeze invariants don't need to know about pause. Auto-cleared
    // by every newGame / newGameReplay / setFixedLayout call.
    [[nodiscard]] bool isPaused() const noexcept;
    void setPaused(bool paused);

    // True if the user placed at least one flag during the current game. Stays
    // true even if the flag is later cycled off — it tracks whether a flag was
    // ever used, not whether one is currently set. Reset by every newGame /
    // newGameReplay / setFixedLayout. Used by MainWindow to credit no-flag
    // wins toward the per-difficulty no-flag best-time record.
    [[nodiscard]] bool anyFlagPlaced() const noexcept;

    // Board value (3BV): the minimum number of left-clicks required to clear
    // the current board, assuming optimal play with no flags or chords. Each
    // connected zero-count region (an "opening") contributes 1, plus one for
    // every numbered cell that is not adjacent to any zero. Computed once when
    // mines are placed and cached. Returns 0 before mines have been placed
    // (i.e. while still in Ready before the first click). Used by MainWindow
    // to surface the canonical Minesweeper speedrun efficiency metric (3BV/s)
    // on the win dialog.
    [[nodiscard]] int boardValue() const noexcept;

    // Useful-click count: number of user gestures during the current game
    // that revealed at least one cell. Counts left-click reveals (one per
    // gesture, not per cascaded flood-cell), chord clicks that opened ≥1
    // neighbour, and the keyboard equivalents (Space/Enter on unopened,
    // Space/Enter/D chord on opened numbered). Right-click flag toggles,
    // unsatisfied chords, and clicks on already-opened or flagged cells
    // never count. Reset by every newGame / newGameReplay / setFixedLayout.
    // MainWindow uses this together with boardValue() to render the
    // canonical Minesweeper efficiency metric (3BV / clicks · 100) on the
    // win dialog.
    [[nodiscard]] int userClicks() const noexcept;

    // Live count of flags currently placed on the board. Reflects user gestures
    // only while the game is Ready or Playing — the win-path auto-flag pass
    // (flagAllMines) inflates the count after the state flips to Won, so
    // callers reading this on a win get the post-celebration total. The loss
    // path does not auto-flag, so reading this on a loss gives the user's
    // actual flag count at the moment of explosion. Reset by every newGame /
    // newGameReplay / setFixedLayout.
    [[nodiscard]] int flagsPlaced() const noexcept;

    // Fraction of safe (non-mine) cells revealed so far, expressed as an integer
    // 0-100. Round-half-up. Useful as an end-of-game progress hint on losses —
    // a player who exploded after revealing 87% of the board sees that they
    // were almost there. Stays at the value reached at the moment of explosion
    // (m_openedSafeCount is not touched on loss). Reads 100 after a win.
    [[nodiscard]] int safePercentCleared() const noexcept;

    void setMineCountLabel(QLabel *label);

    // Sweep the live board and reset any Question-marked cell to None. Used when
    // the user disables question marks mid-game so no cell is left stuck in `?`.
    void clearAllQuestionMarks();

    // Sweep the live board and re-apply number-colour styles on every opened
    // numbered cell. Used when the user toggles the colour-blind palette
    // mid-game so the switch is visible immediately on the already-revealed
    // board.
    void refreshAllNumberStyles();

    // Test helper: deterministic layout with explicit mine positions.
    void setFixedLayout(std::uint32_t width, std::uint32_t height, const std::vector<std::pair<std::uint32_t, std::uint32_t>> &minePositions);

    // Test helper: direct access for fixture-based tests.
    [[nodiscard]] MineButton *cellAt(std::uint32_t row, std::uint32_t col) const;

  protected:
    // Keyboard navigation. MineField installs itself as an event filter on
    // every MineButton so it can intercept key events before QAbstractButton
    // eats Space/Enter (for click activation) or arrow keys (for button-group
    // navigation). Centralising in one place keeps MineButton back-pointer-free.
    bool eventFilter(QObject *watched, QEvent *event) override;

  signals:
    void gameStarted();
    void gameWon();
    void gameLost(std::uint32_t row, std::uint32_t col);
    void mineCountChanged(int remaining);
    // Forwarded from MineButton::pressStart/pressEnd so MainWindow can drive
    // the tension smiley without needing per-cell wiring. Cell-agnostic on
    // purpose — the indicator doesn't care which cell is being held.
    void cellInteractionStarted();
    void cellInteractionEnded();

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
    int compute3BV() const;
    void revealAllMines();
    void flagAllMines();
    void freezeAllCells();
    void updateMineCountLabel();
    void checkWin();
    bool handleCellKey(MineButton *cell, int key);
    void focusCell(std::uint32_t row, std::uint32_t col);

    QGridLayout *m_grid{nullptr};
    std::vector<std::vector<MineButton *>> m_buttons;
    QPointer<QLabel> m_mineCountLabel;
    QPointer<QFrame> m_pauseOverlay;

    Difficulty m_difficulty{Beginner};
    GameState m_state{GameState::Ready};
    std::uint32_t m_openedSafeCount{0};
    int m_flagCount{0};
    bool m_minesPlaced{false};
    bool m_paused{false};
    bool m_anyFlagPlaced{false};
    int m_boardValue{0};
    int m_userClicks{0};
    std::vector<std::pair<std::uint32_t, std::uint32_t>> m_lastMinePositions;
};

#endif // MINEFIELD_H
