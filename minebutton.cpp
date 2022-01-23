#include "minebutton.h"

#include <QSizePolicy>

MineButton::MineButton(QWidget *parent)
    : QPushButton{parent}
{
    this->setStyleSheet("border: 1px solid black; background: white");
    this->setFixedSize(30,30);
    //this->setMinimumSize(30,30);
    //this->setSizeIncrement(5,5);
    //this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}
