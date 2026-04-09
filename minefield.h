#ifndef MINEFIELD_H
#define MINEFIELD_H

#include "minebutton.h"

#include <QGridLayout>
#include <QLabel>
#include <QWidget>

class MineField : public QWidget
{
    Q_OBJECT
  public:
    explicit MineField(QWidget *parent = nullptr);
    void incrementflagCount();
    void getMineCountLabel(QLabel *label);

    // Accessors used by unit tests
    MineButton *getButton(uint i, uint j) { return MButtons[i][j]; }
    int getFlagCount() const { return flagCount; }
    static constexpr uint getWidth() { return Width; }
    static constexpr uint getHeight() { return Height; }
    static constexpr int getMineCount() { return MineCount; }

  public slots:
    void checkNeighbours(uint i, uint j);

  private:
    void fillMines();
    void fillNumbers();

    QGridLayout grid{this};
    static constexpr uint Width = 15;
    static constexpr uint Height = 15;
    static constexpr int MineCount = 20;
    MineButton *MButtons[Height][Width];
    int flagCount = 0;
    QLabel *mineCountLabel;
};

#endif // MINEFIELD_H
