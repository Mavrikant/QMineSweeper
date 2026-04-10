#include "minebutton.h"

#include "minefield.h"

#include <QColor>
#include <QFont>
#include <QIcon>
#include <QMouseEvent>
#include <QSizePolicy>
#include <QString>

namespace
{
constexpr int kCellSize = 30;
constexpr int kIconSize = 32;
constexpr const char *kEvenCellStyle = "border: 0px; background: rgba(162, 209, 73, 1);";
constexpr const char *kOddCellStyle = "border: 0px; background: rgba(162, 209, 73, 0.8);";
constexpr const char *kEvenOpenedStyle = "border: 0px; background: rgba(215, 184, 153, 1);";
constexpr const char *kOddOpenedStyle = "border: 0px; background: rgba(215, 184, 153, 0.8);";
constexpr const char *kMineStyle = "border: 0px; background: rgba(255, 209, 73, 1);";

QColor numberColor(std::uint32_t number)
{
    switch (number)
    {
    case 1:
        return {2, 0, 251};
    case 2:
        return {1, 126, 0};
    case 3:
        return {251, 3, 1};
    case 4:
        return {1, 1, 128};
    case 5:
        return {125, 0, 1};
    case 6:
        return {1, 126, 125};
    case 7:
        return {0, 0, 0};
    case 8:
        return {128, 128, 128};
    default:
        return {};
    }
}
} // namespace

MineButton::MineButton(std::uint32_t x, std::uint32_t y, QWidget *parent) : QPushButton{parent}, m_Field{qobject_cast<MineField *>(parent)}, m_x{x}, m_y{y}
{
    setFixedSize(kCellSize, kCellSize);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    setFont(QFont{"Arial", 12, QFont::Bold});
    setStyleSheet(((m_x + m_y) % 2) ? kOddCellStyle : kEvenCellStyle);
}

void MineButton::setNumber(std::uint32_t number)
{
    if (!m_isMined)
    {
        m_number = number;
    }

    const QColor color = numberColor(number);
    setStyleSheet(styleSheet() + QStringLiteral("color: rgb(%1, %2, %3);").arg(color.red()).arg(color.green()).arg(color.blue()));
}

std::uint32_t MineButton::Number() const noexcept { return m_number; }

bool MineButton::isMined() const noexcept { return m_isMined; }

bool MineButton::isOpened() const noexcept { return m_isClicked; }

void MineButton::Open()
{
    m_isClicked = true;
    if (m_number == 0 && !m_isMined)
    {
        emit checkNeighbours(m_x, m_y);
    }

    setStyleSheet(((m_x + m_y) % 2) ? kOddOpenedStyle : kEvenOpenedStyle);

    if (m_isMined)
    {
        setStyleSheet(kMineStyle);
        setIcon(QIcon{":/icons/explosion.png"});
        setIconSize(QSize{kIconSize, kIconSize});
        emit explosion(m_x, m_y);
    }
    else
    {
        setText(m_number == 0 ? QString{} : QString::number(m_number));
        const QColor color = numberColor(m_number);
        setStyleSheet(styleSheet() + QStringLiteral("color: rgb(%1, %2, %3);").arg(color.red()).arg(color.green()).arg(color.blue()));
    }
}

void MineButton::Flag()
{
    if (m_isFlaged || m_isClicked)
    {
        return;
    }
    m_isFlaged = true;
    if (m_Field != nullptr)
    {
        m_Field->incrementflagCount();
    }
    setIcon(QIcon{":/icons/redflag.png"});
}

void MineButton::setMined() noexcept { m_isMined = true; }

void MineButton::mousePressEvent(QMouseEvent *e)
{
    switch (e->button())
    {
    case Qt::LeftButton:
        Open();
        break;
    case Qt::RightButton:
        Flag();
        break;
    default:
        break;
    }
}
