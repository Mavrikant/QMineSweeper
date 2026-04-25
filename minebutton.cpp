#include "minebutton.h"

#include <QColor>
#include <QCursor>
#include <QFont>
#include <QIcon>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QSizePolicy>
#include <QString>

namespace
{
constexpr int kCellSize = MineButton::CellSize;
constexpr int kIconSize = 22;
constexpr const char *kEvenCellStyle = "border: 0px; background: rgba(162, 209, 73, 1);";
constexpr const char *kOddCellStyle = "border: 0px; background: rgba(162, 209, 73, 0.85);";
constexpr const char *kEvenOpenedStyle = "border: 0px; background: rgba(215, 184, 153, 1);";
constexpr const char *kOddOpenedStyle = "border: 0px; background: rgba(215, 184, 153, 0.85);";
constexpr const char *kMineStyle = "border: 0px; background: rgba(255, 80, 60, 1);";
constexpr const char *kMineRevealStyle = "border: 0px; background: rgba(255, 209, 73, 1);";
constexpr const char *kWrongFlagStyle = "border: 0px; background: rgba(200, 80, 80, 0.9);";

bool g_questionMarksEnabled = true;

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
    // Explicit StrongFocus so keyboard navigation works uniformly across
    // platforms (macOS defaults to Qt::TabFocus for push buttons, which
    // would make arrow-key navigation inconsistent with Linux/Windows).
    setFocusPolicy(Qt::StrongFocus);
    applyBaseStyle();
}

std::uint32_t MineButton::row() const noexcept { return m_row; }

std::uint32_t MineButton::col() const noexcept { return m_col; }

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

bool MineButton::isFlagged() const noexcept { return m_marker == CellMarker::Flag; }

bool MineButton::isQuestion() const noexcept { return m_marker == CellMarker::Question; }

CellMarker MineButton::marker() const noexcept { return m_marker; }

bool MineButton::isOpened() const noexcept { return m_isClicked; }

void MineButton::setCellEnabled(bool enabled) noexcept
{
    m_enabled = enabled;
    setCursor(enabled ? Qt::PointingHandCursor : Qt::ArrowCursor);
}

void MineButton::renderMarker()
{
    switch (m_marker)
    {
    case CellMarker::None:
        setIcon(QIcon{});
        setText(QString{});
        break;
    case CellMarker::Flag:
        setIcon(QIcon{":/icons/redflag.png"});
        setIconSize(QSize{kIconSize, kIconSize});
        setText(QString{});
        break;
    case CellMarker::Question:
        setIcon(QIcon{});
        setText(QStringLiteral("?"));
        // Slightly dimmer than flag to read as annotation.
        setStyleSheet(styleSheet() + QStringLiteral("color: rgb(60, 60, 60);"));
        break;
    }
}

void MineButton::Open()
{
    // Flags block reveal; question marks do not.
    if (m_isClicked || m_marker == CellMarker::Flag)
    {
        return;
    }

    emit cellPressed(m_row, m_col);

    // Revealing clears any question-mark annotation.
    m_marker = CellMarker::None;
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
    if (m_isClicked || m_marker == CellMarker::Flag)
    {
        return;
    }
    const bool wasFlagged = (m_marker == CellMarker::Flag);
    m_marker = CellMarker::Flag;
    renderMarker();
    if (!wasFlagged)
    {
        emit flagToggled(m_row, m_col, true);
    }
}

void MineButton::cycleMarker()
{
    if (m_isClicked)
    {
        return;
    }
    const CellMarker prev = m_marker;
    switch (prev)
    {
    case CellMarker::None:
        m_marker = CellMarker::Flag;
        break;
    case CellMarker::Flag:
        m_marker = g_questionMarksEnabled ? CellMarker::Question : CellMarker::None;
        break;
    case CellMarker::Question:
        m_marker = CellMarker::None;
        break;
    }
    renderMarker();
    // flagToggled fires whenever the Flag state itself changes.
    if (prev == CellMarker::Flag)
    {
        emit flagToggled(m_row, m_col, false);
    }
    else if (m_marker == CellMarker::Flag)
    {
        emit flagToggled(m_row, m_col, true);
    }
}

void MineButton::clearQuestion()
{
    if (m_marker == CellMarker::Question)
    {
        m_marker = CellMarker::None;
        // Reset the stylesheet so the dim-color rule added by renderMarker()'s
        // Question branch does not persist.
        applyBaseStyle();
        renderMarker();
    }
}

void MineButton::setQuestionMarksEnabled(bool enabled) noexcept { g_questionMarksEnabled = enabled; }

bool MineButton::questionMarksEnabled() noexcept { return g_questionMarksEnabled; }

void MineButton::mousePressEvent(QMouseEvent *e)
{
    if (!m_enabled)
    {
        return;
    }

    const bool leftAndRight = (e->buttons() & (Qt::LeftButton | Qt::RightButton)) == (Qt::LeftButton | Qt::RightButton);

    // Tension-smiley trigger — fire before any reveal/chord logic so the 😮
    // face paints while the button is still held, not only after release.
    // Right-click-only presses are deliberately excluded: flag-cycling should
    // not change the header indicator.
    if (e->button() == Qt::LeftButton || e->button() == Qt::MiddleButton || leftAndRight)
    {
        emit pressStart();
    }

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
        if (m_marker != CellMarker::Flag && !m_isClicked)
        {
            // Emit before Open() so cascade-driven Open()s don't see the
            // signal — userClick is gesture-scoped, cellOpened is per-cell.
            emit userClick();
            Open();
        }
        break;
    case Qt::RightButton:
        cycleMarker();
        break;
    default:
        break;
    }
}

void MineButton::mouseReleaseEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
    // Always emit on release — MainWindow tracks tension with a single flag,
    // so an unmatched pressEnd (e.g. after a right-click-only press that never
    // emitted pressStart) is a harmless no-op.
    emit pressEnd();
}

void MineButton::paintEvent(QPaintEvent *e)
{
    QPushButton::paintEvent(e);
    if (!hasFocus())
    {
        return;
    }
    // Keyboard-focus indicator. The cell stylesheet uses border: 0 and suppresses
    // the native focus ring; draw a thin inset rectangle so keyboard users can
    // see which cell is selected.
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);
    p.setPen(QPen(QColor(40, 90, 200), 2));
    p.setBrush(Qt::NoBrush);
    p.drawRect(1, 1, width() - 2, height() - 2);
}
