#include "minefield.h"
#include "minebutton.h"

MineField::MineField(QWidget *parent) : QWidget{parent}
{

    for (int i = 0; i < 15; ++i)
    {

        for (int j = 0; j < 15; ++j)
        {
            MineButton *newButton = new MineButton(this);
            grid.addWidget(newButton, i, j);
            grid.setSpacing(0);
        }
    }
    setLayout(&grid);
}
