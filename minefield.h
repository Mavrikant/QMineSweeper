#ifndef MINEFIELD_H
#define MINEFIELD_H

#include "minebutton.h"

#include <QGridLayout>
#include <QWidget>

class MineField : public QWidget
{
    Q_OBJECT
  public:
    explicit MineField(QWidget *parent = nullptr);

  public slots:
    void checkNeighbours(uint i, uint j);
  signals:

  private:
    void fillMines();
    void fillNumbers();
    QGridLayout grid{this};
    static constexpr uint Width = 15;
    static constexpr uint Height = 15;
    MineButton *MButtons[Height][Width];
};

#endif // MINEFIELD_H
