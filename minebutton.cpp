#include "minebutton.h"

#include <QSizePolicy>

MineButton::MineButton(uint x, uint y, QWidget *parent) : QPushButton{parent}
{
    m_x = x;
    m_y = y;
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

uint MineButton::Number() { return this->number; }

bool MineButton::isMined() { return m_isMined; }

bool MineButton::isOpened() { return m_isClicked; }

void MineButton::Open()
{
    m_isClicked = true;
    if (number == 0 && m_isMined == false)
    {
        emit checkNeighbours(m_x, m_y);
    }
    else if (m_isMined == false)
    {
        emit explosion(m_x, m_y);
    }
}

void MineButton::setMined() { m_isMined = true; }

void MineButton::paintEvent(QPaintEvent *)
{
    if (m_isClicked)
    {
        this->setStyleSheet("border: 0px solid grey;; background: yellow");
        if (m_isMined)
        {
            this->setText("#");
        }
        else if (number == 0)
        {
            this->setText("");
        }
        else
        {
            this->setText(QString::number(number));
        }
    }
    else
    {
        this->setStyleSheet("border: 1px solid grey;; background: orange");
    }

    QPushButton::paintEvent(nullptr);
}

void MineButton::mousePressEvent(QMouseEvent *e) { Open(); }
