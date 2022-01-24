#include "minebutton.h"

#include <QSizePolicy>

MineButton::MineButton(QWidget *parent) : QPushButton{parent}
{

    // this->setStyleSheet("border: 1px solid black; background: white");
    this->setFixedSize(30, 30);
    // this->setMinimumSize(30,30);
    // this->setSizeIncrement(5,5);
    // this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void MineButton::setNumber(uint number)
{
    if (!m_isMined)
    {
        this->number = number;
    }
}

bool MineButton::isMined() { return m_isMined; }

void MineButton::setMined() { m_isMined = true; }

void MineButton::paintEvent(QPaintEvent *)
{
    if (m_isClicked)
    {
        if (m_isMined)
        {
            this->setText("*");
        }
        else
        {
            this->setText(QString::number(number));
        }
    }

    QPushButton::paintEvent(nullptr);
}

void MineButton::mousePressEvent(QMouseEvent *e) { m_isClicked = true; }
