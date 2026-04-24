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
    void setCellEnabled(bool enabled) noexcept;
    void revealAsMine();
    void revealAsWrongFlag();
    void autoFlag();
    void clearQuestion();

    // App-wide: when false, right-click cycles None → Flag → None (skips Question).
    // MineButton has no back-pointer by design, so the toggle lives as static state.
    static void setQuestionMarksEnabled(bool enabled) noexcept;
    [[nodiscard]] static bool questionMarksEnabled() noexcept;

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

  protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;

  private:
    void cycleMarker();
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
