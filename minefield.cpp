#include "minefield.h"
#include "minebutton.h"

MineField::MineField(QWidget *parent) : QWidget{parent}
{
    for (uint i = 0; i < Height; ++i)
    {
        for (uint j = 0; j < Width; ++j)
        {
            MButtons[i][j] = new MineButton(i, j, this);
            connect(MButtons[i][j], &MineButton::checkNeighbours, this, &MineField::checkNeighbours);
            grid.addWidget(MButtons[i][j], i, j);
            grid.setSpacing(0);
        }
    }
    setLayout(&grid);
    fillMines();
    fillNumbers();
}

void MineField::fillMines()
{
    for (uint i = 0; i < 20; ++i)
    {
        uint h = rand() % Height;
        uint w = rand() % Width;
        MButtons[h][w]->setMined();
    }
}

void MineField::fillNumbers()
{
    for (int i = 0; i < Height; ++i)
    {
        for (int j = 0; j < Width; ++j)
        {
            uint minenumber = 0;

            for (int a = -1; a <= 1; ++a)
            {
                for (int b = -1; b <= 1; ++b)
                {
                    if (i + a >= 0 && i + a < Height && j + b >= 0 && j + b < Width)
                    {
                        minenumber += MButtons[i + a][j + b]->isMined();
                    }
                }
            }
            MButtons[i][j]->setNumber(minenumber);
        }
    }
}

void MineField::checkNeighbours(uint i, uint j)
{
    for (int a = -1; a <= 1; ++a)
    {
        for (int b = -1; b <= 1; ++b)
        {
            if (i + a >= 0 && i + a < Height && j + b >= 0 && j + b < Width)
            {
                if (MButtons[i + a][j + b]->isOpened() == false)
                {
                    MButtons[i + a][j + b]->Open();
                }
            }
        }
    }
    update();
}
