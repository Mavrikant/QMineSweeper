#ifndef MINEBUTTON_H
#define MINEBUTTON_H

#include <QPushButton>

#include <cstdint>

enum class CellMarker : std::uint8_t
{
    None,
    Flag,
    Question
};

class MineButton : public QPushButton
{
    Q_OBJECT
  public:
    static constexpr int CellSize = 30;

    explicit MineButton(std::uint32_t row, std::uint32_t col, QWidget *parent = nullptr);

    [[nodiscard]] std::uint32_t row() const noexcept;
    [[nodiscard]] std::uint32_t col() const noexcept;

    void setNumber(std::uint32_t number);
    [[nodiscard]] std::uint32_t Number() const noexcept;

    [[nodiscard]] bool isMined() const noexcept;
    void setMined() noexcept;
    void clearMined() noexcept;

    [[nodiscard]] bool isFlagged() const noexcept;
    [[nodiscard]] bool isQuestion() const noexcept;
    [[nodiscard]] CellMarker marker() const noexcept;
    [[nodiscard]] bool isOpened() const noexcept;

    void Open();
    // Advance the marker state (None → Flag → Question → None, or
    // None → Flag → None when question marks are disabled). Emits
    // flagToggled on Flag-on / Flag-off transitions. Called internally
    // on right-click; also used by the keyboard-navigation path.
    void cycleMarker();
    void setCellEnabled(bool enabled) noexcept;
    void revealAsMine();
    void revealAsWrongFlag();
    void autoFlag();
    void clearQuestion();

    // App-wide: when false, right-click cycles None → Flag → None (skips Question).
    // MineButton has no back-pointer by design, so the toggle lives as static state.
    static void setQuestionMarksEnabled(bool enabled) noexcept;
    [[nodiscard]] static bool questionMarksEnabled() noexcept;

    // App-wide: when true, opened numbered cells render with the Okabe-Ito-
    // derived palette instead of the classic Minesweeper 1–8 colours. The
    // classic palette collapses 2/3 (green/red) and 5 (dark red) for players
    // with red-green colour vision deficiency; the alternate palette keeps
    // every digit distinguishable under deuteranopia and protanopia. Same
    // static-state pattern as question-marks — MineButton has no back-pointer,
    // so the toggle is app-global. Call `refreshNumberStyle()` on opened
    // cells to re-render with the new palette mid-game.
    static void setColorBlindPaletteEnabled(bool enabled) noexcept;
    [[nodiscard]] static bool colorBlindPaletteEnabled() noexcept;

    // Re-apply the opened-cell stylesheet (background + number colour) so a
    // toggle of the colour-blind palette is reflected immediately on cells
    // that were opened before the change. No-op on unopened or mined cells —
    // mined cells render their explosion/reveal styles that don't depend on
    // the number palette.
    void refreshNumberStyle();

  signals:
    void cellPressed(std::uint32_t row, std::uint32_t col);
    void cellOpened(std::uint32_t row, std::uint32_t col);
    void explosion(std::uint32_t row, std::uint32_t col);
    void checkNeighbours(std::uint32_t row, std::uint32_t col);
    // Emitted when the flag flag transitions — i.e. Flag-on and Flag-off.
    // A cycle through Question fires flagToggled(..., false) as it leaves Flag
    // and does NOT fire again when it returns to None from Question.
    void flagToggled(std::uint32_t row, std::uint32_t col, bool flagged);
    void chordRequested(std::uint32_t row, std::uint32_t col);
    // Fires while a left/middle/chord mouse button is held down on the cell
    // (before any reveal/chord is processed). Right-click-only presses do not
    // fire pressStart — they just cycle the marker without any hold feedback.
    // MainWindow uses these to drive the classic 😮 "tension" smiley.
    void pressStart();
    void pressEnd();
    // Mouse-driven user gesture that will reveal this cell. Emitted from
    // mousePressEvent on a left-click that is committing to a fresh Open()
    // (cell unopened and not flag-marked); not emitted from flood-fill or
    // chord-neighbour Open() calls. MineField listens to score the
    // efficiency-metric click count without overcounting cascades.
    void userClick();

  protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void paintEvent(QPaintEvent *e) override;

  private:
    void applyBaseStyle();
    void applyOpenedStyle();
    void renderMarker();

    std::uint32_t m_row{0};
    std::uint32_t m_col{0};
    std::uint32_t m_number{0};
    CellMarker m_marker{CellMarker::None};
    bool m_isMined{false};
    bool m_isClicked{false};
    bool m_enabled{true};
};

#endif // MINEBUTTON_H
