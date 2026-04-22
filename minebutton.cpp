#include "minebutton.h"

#include <QColor>
#include <QCursor>
#include <QFont>
#include <QIcon>
#include <QMouseEvent>
#include <QSizePolicy>
#include <QString>

namespace
{
constexpr int kCellSize = 30;
constexpr int kIconSize = 22;
constexpr const char *kEvenCellStyle = "border: 0px; background: rgba(162, 209, 73, 1);";
constexpr const char *kOddCellStyle = "border: 0px; background: rgba(162, 209, 73, 0.85);";
constexpr const char *kEvenOpenedStyle = "border: 0px; background: rgba(215, 184, 153, 1);";
constexpr const char *kOddOpenedStyle = "border: 0px; background: rgba(215, 184, 153, 0.85);";
constexpr const char *kMineStyle = "border: 0px; background: rgba(255, 80, 60, 1);";
constexpr const char *kMineRevealStyle = "border: 0px; background: rgba(255, 209, 73, 1);";
constexpr const char *kWrongFlagStyle = "border: 0px; background: rgba(200, 80, 80, 0.9);";

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

MineButton::MineButton(std::uint32_t row, std::uint32_t col, QWidget *parent) : QPushButton{parent}, m_row{row}, m_col{col}
{
    setFixedSize(kCellSize, kCellSize);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    setFont(QFont{"Arial", 12, QFont::Bold});
    setCursor(Qt::PointingHandCursor);
    applyBaseStyle();
}

void MineButton::applyBaseStyle() { setStyleSheet(((m_row + m_col) % 2) ? kOddCellStyle : kEvenCellStyle); }

void MineButton::applyOpenedStyle() { setStyleSheet(((m_row + m_col) % 2) ? kOddOpenedStyle : kEvenOpenedStyle); }

void MineButton::setNumber(std::uint32_t number)
{
    if (m_isMined)
    {
        return;
    }
    m_number = number;
}

std::uint32_t MineButton::Number() const noexcept { return m_number; }

bool MineButton::isMined() const noexcept { return m_isMined; }

void MineButton::setMined() noexcept { m_isMined = true; }

void MineButton::clearMined() noexcept { m_isMined = false; }

bool MineButton::isFlagged() const noexcept { return m_isFlagged; }

bool MineButton::isOpened() const noexcept { return m_isClicked; }

void MineButton::setCellEnabled(bool enabled) noexcept
{
    m_enabled = enabled;
    setCursor(enabled ? Qt::PointingHandCursor : Qt::ArrowCursor);
}

void MineButton::Open()
{
    if (m_isClicked || m_isFlagged)
    {
        return;
    }

    emit cellPressed(m_row, m_col);

    m_isClicked = true;
    applyOpenedStyle();

    if (m_isMined)
    {
        setStyleSheet(kMineStyle);
        setIcon(QIcon{":/icons/explosion.png"});
        setIconSize(QSize{kIconSize, kIconSize});
        emit explosion(m_row, m_col);
        return;
    }

    setText(m_number == 0 ? QString{} : QString::number(m_number));
    const QColor color = numberColor(m_number);
    setStyleSheet(styleSheet() + QStringLiteral("color: rgb(%1, %2, %3);").arg(color.red()).arg(color.green()).arg(color.blue()));

    emit cellOpened(m_row, m_col);

    if (m_number == 0)
    {
        emit checkNeighbours(m_row, m_col);
    }
}

void MineButton::revealAsMine()
{
    if (m_isClicked)
    {
        return;
    }
    m_isClicked = true;
    setStyleSheet(kMineRevealStyle);
    setIcon(QIcon{":/icons/explosion.png"});
    setIconSize(QSize{kIconSize, kIconSize});
}

void MineButton::revealAsWrongFlag()
{
    setStyleSheet(kWrongFlagStyle);
    setIcon(QIcon{":/icons/redflag.png"});
    setIconSize(QSize{kIconSize, kIconSize});
    setText(QStringLiteral("×"));
}

void MineButton::autoFlag()
{
    if (m_isClicked)
    {
        return;
    }
    if (!m_isFlagged)
    {
        m_isFlagged = true;
        setIcon(QIcon{":/icons/redflag.png"});
        setIconSize(QSize{kIconSize, kIconSize});
        emit flagToggled(m_row, m_col, true);
    }
}

void MineButton::Flag()
{
    if (m_isClicked)
    {
        return;
    }
    if (m_isFlagged)
    {
        m_isFlagged = false;
        setIcon(QIcon{});
        emit flagToggled(m_row, m_col, false);
    }
    else
    {
        m_isFlagged = true;
        setIcon(QIcon{":/icons/redflag.png"});
        setIconSize(QSize{kIconSize, kIconSize});
        emit flagToggled(m_row, m_col, true);
    }
}

void MineButton::mousePressEvent(QMouseEvent *e)
{
    if (!m_enabled)
    {
        return;
    }

    const bool leftAndRight = (e->buttons() & (Qt::LeftButton | Qt::RightButton)) == (Qt::LeftButton | Qt::RightButton);

    if (e->button() == Qt::MiddleButton || leftAndRight)
    {
        if (m_isClicked)
        {
            emit chordRequested(m_row, m_col);
        }
        return;
    }

    switch (e->button())
    {
    case Qt::LeftButton:
        if (!m_isFlagged)
        {
            Open();
        }
        break;
    case Qt::RightButton:
        Flag();
        break;
    default:
        break;
    }
}
