#ifndef MINEBUTTON_H
#define MINEBUTTON_H

#include <QPushButton>

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
    Flag
};

class MineButton : public QPushButton
{
    Q_OBJECT
  public:
    explicit MineButton(uint x, uint y, QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *e) override;
    void setNumber(uint number);
    bool isMined();
    void setMined();

    uint Number();
    bool isOpened();
    void Open();
  signals:
    void checkNeighbours(uint m_x, uint m_y);
    void explosion(uint m_x, uint m_y);

  private:
    uint m_x = 0;
    uint m_y = 0;
    bool m_isMined = false;
    bool m_isClicked = false;
    uint number = 0;
};

#endif // MINEBUTTON_H
