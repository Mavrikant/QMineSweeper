#ifndef MINEBUTTON_H
#define MINEBUTTON_H

#include <QPushButton>

#include <cstdint>

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
    [[nodiscard]] bool isOpened() const noexcept;

    void Open();
    void setCellEnabled(bool enabled) noexcept;
    void revealAsMine();
    void revealAsWrongFlag();
    void autoFlag();

  signals:
    void cellPressed(std::uint32_t row, std::uint32_t col);
    void cellOpened(std::uint32_t row, std::uint32_t col);
    void explosion(std::uint32_t row, std::uint32_t col);
    void checkNeighbours(std::uint32_t row, std::uint32_t col);
    void flagToggled(std::uint32_t row, std::uint32_t col, bool flagged);
    void chordRequested(std::uint32_t row, std::uint32_t col);

  protected:
    void mousePressEvent(QMouseEvent *e) override;

  private:
    void Flag();
    void applyBaseStyle();
    void applyOpenedStyle();

    std::uint32_t m_row{0};
    std::uint32_t m_col{0};
    std::uint32_t m_number{0};
    bool m_isMined{false};
    bool m_isClicked{false};
    bool m_isFlagged{false};
    bool m_enabled{true};
};

#endif // MINEBUTTON_H
