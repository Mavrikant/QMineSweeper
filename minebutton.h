#ifndef MINEBUTTON_H
#define MINEBUTTON_H

#include <QPushButton>

#include <cstdint>

class MineField;

enum class MineButtonState
{
    Empty,
    Number1,
    Number2,
    Number3,
    Number4,
    Number5,
    Number6,
    Number7,
    Mine,
    Flaged
};

class MineButton : public QPushButton
{
    Q_OBJECT
  public:
    explicit MineButton(std::uint32_t x, std::uint32_t y, QWidget *parent);

    void setNumber(std::uint32_t number);
    [[nodiscard]] std::uint32_t Number() const noexcept;

    [[nodiscard]] bool isMined() const noexcept;
    void setMined() noexcept;

    [[nodiscard]] bool isOpened() const noexcept;
    void Open();

  signals:
    void checkNeighbours(std::uint32_t m_x, std::uint32_t m_y);
    void explosion(std::uint32_t m_x, std::uint32_t m_y);

  protected:
    void mousePressEvent(QMouseEvent *e) override;

  private:
    void Flag();

    MineField *m_Field{nullptr};
    std::uint32_t m_x{0};
    std::uint32_t m_y{0};
    std::uint32_t m_number{0};
    bool m_isMined{false};
    bool m_isClicked{false};
    bool m_isFlaged{false};
};

#endif // MINEBUTTON_H
