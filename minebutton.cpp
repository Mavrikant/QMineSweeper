#include "minebutton.h"
#include "minefield.h"
#include <QMouseEvent>
#include <QSizePolicy>

MineButton::MineButton(uint x, uint y, QWidget *parent) : QPushButton{parent}
{
    this->setFixedSize(30, 30);
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    this->m_Field = reinterpret_cast<MineField *>(parent);
    m_x = x;
    m_y = y;
    QFont f("Arial", 12, QFont::Bold);
    setFont(f);
    this->setStyleSheet("border: 0px;");
    this->setStyleSheet(this->styleSheet() + "background: rgba(162, 209, 73, 1);");
    if ((m_x + m_y) % 2)
    {
        this->setStyleSheet(this->styleSheet().replace("1);", "0.8);"));
    }
}

void MineButton::setNumber(uint number)
{
    if (!m_isMined)
    {
        this->number = number;
    }

    QColor color;

    switch (number)
    {
    case 1:
    {
        color = QColor::fromRgb(2, 0, 251);
        break;
    }

    case 2:
    {
        color = QColor::fromRgb(1, 126, 0);
        break;
    }
    case 3:
    {
        color = QColor::fromRgb(251, 3, 1);
        break;
    }
    case 4:
    {
        color = QColor::fromRgb(1, 1, 128);
        break;
    }
    case 5:

    {
        color = QColor::fromRgb(125, 0, 1);
        break;
    }
    case 6:

    {
        color = QColor::fromRgb(1, 126, 125);
        break;
    }
    case 7:

    {
        color = QColor::fromRgb(0, 0, 0);
        break;
    }
    case 8:
    {
        color = QColor::fromRgb(128, 128, 128);
        break;
    }
    default:
    {
        break;
    }
    }

    this->setStyleSheet(this->styleSheet() + QString("color: rgb(%1, %2, %3);").arg(color.red()).arg(color.green()).arg(color.blue()));
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

    this->setStyleSheet(this->styleSheet() + "background: rgba(215, 184, 153, 1);");
    if (m_isMined)
    {
        this->setText("#");
    }
    else
    {
        this->setText(number == 0 ? "" : QString::number(number));
    }
    if ((m_x + m_y) % 2)
    {
        this->setStyleSheet(this->styleSheet().replace("1);", "0.8);"));
    }
}

void MineButton::Flag()
{
    m_isFlaged = true;
    m_Field->incrementflagCount();
    this->setStyleSheet(this->styleSheet() + "background: rgba(255, 209, 73, 1);");
}

void MineButton::setMined() { m_isMined = true; }

void MineButton::mousePressEvent(QMouseEvent *e)
{
    switch (e->button())
    {
    case Qt::LeftButton:
    {
        Open();
        break;
    }
    case Qt::RightButton:
    {
        Flag();
        break;
    }
    default:
    {
        // Do nothing
        break;
    }
    }
}
