#ifndef MINEBUTTON_H
#define MINEBUTTON_H

#include <QPushButton>

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
    explicit MineButton(uint x, uint y, QWidget *parent);
    void mousePressEvent(QMouseEvent *e);
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
    MineField *m_Field;
    uint m_x = 0;
    uint m_y = 0;
    bool m_isMined = false;
    bool m_isClicked = false;
    bool m_isFlaged = false;
    uint number = 0;
    void Flag();
};

#endif // MINEBUTTON_H
