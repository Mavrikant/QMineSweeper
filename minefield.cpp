#include "minefield.h"

#include <QCoreApplication>
#include <QEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QSizePolicy>
#include <QVBoxLayout>

#include <algorithm>
#include <random>

namespace
{
bool inBounds(int row, int col, std::uint32_t rows, std::uint32_t cols) { return row >= 0 && row < static_cast<int>(rows) && col >= 0 && col < static_cast<int>(cols); }
} // namespace

MineField::MineField(QWidget *parent) : QWidget{parent}
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_grid = new QGridLayout(this);
    m_grid->setSpacing(0);
    m_grid->setContentsMargins(0, 0, 0, 0);
    newGame(Beginner);
}

void MineField::newGame() { newGame(m_difficulty); }

void MineField::newGame(Difficulty diff)
{
    m_difficulty = diff;
    m_state = GameState::Ready;
    m_openedSafeCount = 0;
    m_flagCount = 0;
    m_minesPlaced = false;
    m_paused = false;
    m_anyFlagPlaced = false;
    m_lastMinePositions.clear();

    clearGrid();
    buildGrid();
    if (m_pauseOverlay)
    {
        m_pauseOverlay->hide();
    }
    updateMineCountLabel();
    emit mineCountChanged(remainingMines());
}

bool MineField::canReplay() const noexcept { return !m_lastMinePositions.empty(); }

bool MineField::newGameReplay()
{
    if (m_lastMinePositions.empty())
    {
        newGame(m_difficulty);
        return false;
    }

    // Snapshot the mines from the previous game; clearGrid()/buildGrid()
    // rebuild the MineButton grid from scratch, so we need to re-apply the
    // mined flags afterwards, not rely on the old buttons.
    const auto mines = m_lastMinePositions;

    m_state = GameState::Ready;
    m_openedSafeCount = 0;
    m_flagCount = 0;
    m_paused = false;
    m_anyFlagPlaced = false;
    // Mines are already known for this layout — skip first-click placement.
    m_minesPlaced = true;

    clearGrid();
    buildGrid();
    if (m_pauseOverlay)
    {
        m_pauseOverlay->hide();
    }

    for (const auto &p : mines)
    {
        if (p.first < m_difficulty.height && p.second < m_difficulty.width)
        {
            m_buttons[p.first][p.second]->setMined();
        }
    }
    fillNumbers();

    updateMineCountLabel();
    emit mineCountChanged(remainingMines());
    return true;
}

GameState MineField::state() const noexcept { return m_state; }

std::uint32_t MineField::cols() const noexcept { return m_difficulty.width; }

std::uint32_t MineField::rows() const noexcept { return m_difficulty.height; }

std::uint32_t MineField::mineCount() const noexcept { return m_difficulty.mineCount; }

int MineField::remainingMines() const noexcept { return static_cast<int>(m_difficulty.mineCount) - m_flagCount; }

const Difficulty &MineField::difficulty() const noexcept { return m_difficulty; }

MineButton *MineField::cellAt(std::uint32_t row, std::uint32_t col) const
{
    if (row >= m_difficulty.height || col >= m_difficulty.width)
    {
        return nullptr;
    }
    return m_buttons[row][col];
}

void MineField::setMineCountLabel(QLabel *label)
{
    m_mineCountLabel = label;
    updateMineCountLabel();
}

void MineField::updateMineCountLabel()
{
    if (m_mineCountLabel)
    {
        m_mineCountLabel->setNum(remainingMines());
    }
}

void MineField::clearGrid()
{
    for (auto &row : m_buttons)
    {
        for (auto *btn : row)
        {
            if (btn)
            {
                m_grid->removeWidget(btn);
                btn->deleteLater();
            }
        }
    }
    m_buttons.clear();
    setMinimumSize(0, 0);
    setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
}

void MineField::buildGrid()
{
    m_buttons.assign(m_difficulty.height, std::vector<MineButton *>(m_difficulty.width, nullptr));

    for (std::uint32_t r = 0; r < m_difficulty.height; ++r)
    {
        for (std::uint32_t c = 0; c < m_difficulty.width; ++c)
        {
            auto *btn = new MineButton(r, c, this);
            btn->setContentsMargins(0, 0, 0, 0);
            wireButton(btn);
            m_grid->addWidget(btn, static_cast<int>(r), static_cast<int>(c));
            m_buttons[r][c] = btn;
        }
    }
    // Lock the widget to an exact pixel size so the outer QVBoxLayout cannot
    // squish the cells out of square.
    setFixedSize(static_cast<int>(m_difficulty.width) * MineButton::CellSize, static_cast<int>(m_difficulty.height) * MineButton::CellSize);
}

void MineField::wireButton(MineButton *button)
{
    connect(button, &MineButton::cellPressed, this, &MineField::onCellPressed);
    connect(button, &MineButton::cellOpened, this, &MineField::onCellOpened);
    connect(button, &MineButton::explosion, this, &MineField::onMineExploded);
    connect(button, &MineButton::checkNeighbours, this, &MineField::onCheckNeighbours);
    connect(button, &MineButton::flagToggled, this, &MineField::onFlagToggled);
    connect(button, &MineButton::chordRequested, this, &MineField::onChordRequested);
    connect(button, &MineButton::pressStart, this, &MineField::cellInteractionStarted);
    connect(button, &MineButton::pressEnd, this, &MineField::cellInteractionEnded);
    button->installEventFilter(this);
}

bool MineField::eventFilter(QObject *watched, QEvent *event)
{
    // Pause swallows every interaction at the cell boundary. The overlay
    // already absorbs mouse events geometrically, but a focused cell can
    // still receive key events (focus is widget-based, not z-order based)
    // — gating here is the only path that catches both.
    if (m_paused && qobject_cast<MineButton *>(watched))
    {
        switch (event->type())
        {
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
        case QEvent::KeyPress:
        case QEvent::KeyRelease:
            return true;
        default:
            // ShortcutOverride is deliberately NOT swallowed — the global
            // P/Ctrl+Q/F2/etc. shortcuts must keep working while paused.
            break;
        }
    }
    if (event->type() == QEvent::KeyPress)
    {
        if (auto *cell = qobject_cast<MineButton *>(watched))
        {
            auto *ke = static_cast<QKeyEvent *>(event);
            if (handleCellKey(cell, ke->key()))
            {
                return true;
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}

bool MineField::isPaused() const noexcept { return m_paused; }

bool MineField::anyFlagPlaced() const noexcept { return m_anyFlagPlaced; }

void MineField::setPaused(bool paused)
{
    if (m_paused == paused)
    {
        return;
    }
    m_paused = paused;

    if (paused)
    {
        if (!m_pauseOverlay)
        {
            // Lazy-construct the overlay the first time we need it; the rest
            // of the field's lifetime keeps it as a hidden child. Parented to
            // the MineField so it sits on top of every cell in z-order and
            // naturally absorbs mouse clicks aimed at the grid.
            auto *frame = new QFrame(this);
            frame->setFrameShape(QFrame::NoFrame);
            frame->setStyleSheet(QStringLiteral("QFrame { background-color: rgba(0, 0, 0, 140); }"
                                                "QLabel { color: white; font-size: 22px; font-weight: bold;"
                                                " background: transparent; }"));
            auto *layout = new QVBoxLayout(frame);
            layout->setContentsMargins(0, 0, 0, 0);
            auto *label = new QLabel(tr("Paused"), frame);
            label->setObjectName(QStringLiteral("pauseLabel"));
            label->setAlignment(Qt::AlignCenter);
            layout->addWidget(label);
            m_pauseOverlay = frame;
        }
        m_pauseOverlay->setGeometry(rect());
        m_pauseOverlay->raise();
        m_pauseOverlay->show();
    }
    else if (m_pauseOverlay)
    {
        m_pauseOverlay->hide();
    }
}

bool MineField::handleCellKey(MineButton *cell, int key)
{
    if (m_state == GameState::Won || m_state == GameState::Lost)
    {
        // Board is frozen; let arrow keys still move focus so the user can
        // look at cells, but swallow action keys so they can't re-enter logic.
        switch (key)
        {
        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_Left:
        case Qt::Key_Right:
            break;
        default:
            return false;
        }
    }

    const std::uint32_t r = cell->row();
    const std::uint32_t c = cell->col();

    switch (key)
    {
    case Qt::Key_Up:
        if (r > 0)
        {
            focusCell(r - 1, c);
        }
        return true;
    case Qt::Key_Down:
        if (r + 1 < m_difficulty.height)
        {
            focusCell(r + 1, c);
        }
        return true;
    case Qt::Key_Left:
        if (c > 0)
        {
            focusCell(r, c - 1);
        }
        return true;
    case Qt::Key_Right:
        if (c + 1 < m_difficulty.width)
        {
            focusCell(r, c + 1);
        }
        return true;
    case Qt::Key_Space:
    case Qt::Key_Return:
    case Qt::Key_Enter:
        // Space/Enter on an opened numbered cell chords, matching mouse UX
        // (middle-click / left+right). On an unopened non-flag cell it reveals.
        // On a flagged cell it is a deliberate no-op — mirrors left-click.
        if (cell->isOpened())
        {
            onChordRequested(r, c);
        }
        else if (!cell->isFlagged())
        {
            cell->Open();
        }
        return true;
    case Qt::Key_F:
        // Toggle marker, same cycle as right-click.
        cell->cycleMarker();
        return true;
    case Qt::Key_D:
        // Chord-only shortcut for keyboard users — the Space path auto-picks
        // reveal vs. chord based on opened state; D forces chord. Matches the
        // middle-click affordance for players who memorise the opened state.
        if (cell->isOpened())
        {
            onChordRequested(r, c);
        }
        return true;
    default:
        return false;
    }
}

void MineField::focusCell(std::uint32_t row, std::uint32_t col)
{
    if (row >= m_difficulty.height || col >= m_difficulty.width)
    {
        return;
    }
    if (auto *target = m_buttons[row][col])
    {
        target->setFocus(Qt::TabFocusReason);
    }
}

void MineField::fillMines(std::uint32_t safeRow, std::uint32_t safeCol)
{
    std::random_device rd;
    std::mt19937 gen{rd()};
    std::uniform_int_distribution<std::uint32_t> distRow{0, m_difficulty.height - 1};
    std::uniform_int_distribution<std::uint32_t> distCol{0, m_difficulty.width - 1};

    const std::uint32_t totalCells = m_difficulty.width * m_difficulty.height;
    auto isExcluded = [&](std::uint32_t r, std::uint32_t c)
    {
        const int dr = static_cast<int>(r) - static_cast<int>(safeRow);
        const int dc = static_cast<int>(c) - static_cast<int>(safeCol);
        return std::abs(dr) <= 1 && std::abs(dc) <= 1;
    };

    // Count the safe zone; if too few cells remain for requested mines, shrink exclusion to just the pressed cell.
    std::uint32_t excludedCount = 0;
    for (std::uint32_t r = 0; r < m_difficulty.height; ++r)
    {
        for (std::uint32_t c = 0; c < m_difficulty.width; ++c)
        {
            if (isExcluded(r, c))
            {
                ++excludedCount;
            }
        }
    }
    const bool relaxExclusion = totalCells - excludedCount < m_difficulty.mineCount;

    m_lastMinePositions.clear();
    m_lastMinePositions.reserve(m_difficulty.mineCount);

    std::uint32_t placed = 0;
    while (placed < m_difficulty.mineCount)
    {
        const std::uint32_t r = distRow(gen);
        const std::uint32_t c = distCol(gen);
        if (m_buttons[r][c]->isMined())
        {
            continue;
        }
        if (!relaxExclusion && isExcluded(r, c))
        {
            continue;
        }
        if (relaxExclusion && r == safeRow && c == safeCol)
        {
            continue;
        }
        m_buttons[r][c]->setMined();
        m_lastMinePositions.emplace_back(r, c);
        ++placed;
    }
}

void MineField::fillNumbers()
{
    for (std::uint32_t r = 0; r < m_difficulty.height; ++r)
    {
        for (std::uint32_t c = 0; c < m_difficulty.width; ++c)
        {
            std::uint32_t n = 0;
            for (int dr = -1; dr <= 1; ++dr)
            {
                for (int dc = -1; dc <= 1; ++dc)
                {
                    const int nr = static_cast<int>(r) + dr;
                    const int nc = static_cast<int>(c) + dc;
                    if (inBounds(nr, nc, m_difficulty.height, m_difficulty.width))
                    {
                        if (m_buttons[nr][nc]->isMined())
                        {
                            ++n;
                        }
                    }
                }
            }
            m_buttons[r][c]->setNumber(n);
        }
    }
}

void MineField::onCellPressed(std::uint32_t row, std::uint32_t col)
{
    if (m_state != GameState::Ready)
    {
        return;
    }
    if (!m_minesPlaced)
    {
        fillMines(row, col);
        fillNumbers();
        m_minesPlaced = true;
    }
    m_state = GameState::Playing;
    emit gameStarted();
}

void MineField::onCellOpened(std::uint32_t /*row*/, std::uint32_t /*col*/)
{
    if (m_state == GameState::Lost || m_state == GameState::Won)
    {
        return;
    }
    ++m_openedSafeCount;
    checkWin();
}

void MineField::checkWin()
{
    const std::uint32_t totalSafe = m_difficulty.width * m_difficulty.height - m_difficulty.mineCount;
    if (m_state == GameState::Playing && m_openedSafeCount >= totalSafe)
    {
        m_state = GameState::Won;
        flagAllMines();
        freezeAllCells();
        emit gameWon();
    }
}

void MineField::onMineExploded(std::uint32_t row, std::uint32_t col)
{
    if (m_state == GameState::Lost)
    {
        return;
    }
    m_state = GameState::Lost;
    revealAllMines();
    freezeAllCells();
    emit gameLost(row, col);
}

void MineField::onFlagToggled(std::uint32_t /*row*/, std::uint32_t /*col*/, bool flagged)
{
    m_flagCount += flagged ? 1 : -1;
    // Gate the no-flag bookkeeping on a still-live game. The win path calls
    // flagAllMines() which auto-flags every mine after m_state flips to Won —
    // those celebratory flags must not poison anyFlagPlaced. A user flagging
    // before their first left-click is still in GameState::Ready; that counts.
    if (flagged && m_state != GameState::Won && m_state != GameState::Lost)
    {
        // Sticky for the rest of the game. Removing the flag later does not
        // reset this — the no-flag credit is "did you get through without
        // ever leaning on the flag affordance", not "is the board flag-free
        // right now".
        m_anyFlagPlaced = true;
    }
    updateMineCountLabel();
    emit mineCountChanged(remainingMines());
}

void MineField::onCheckNeighbours(std::uint32_t row, std::uint32_t col)
{
    for (int dr = -1; dr <= 1; ++dr)
    {
        for (int dc = -1; dc <= 1; ++dc)
        {
            if (dr == 0 && dc == 0)
            {
                continue;
            }
            const int nr = static_cast<int>(row) + dr;
            const int nc = static_cast<int>(col) + dc;
            if (inBounds(nr, nc, m_difficulty.height, m_difficulty.width))
            {
                auto *n = m_buttons[nr][nc];
                if (!n->isOpened() && !n->isFlagged())
                {
                    n->Open();
                }
            }
        }
    }
}

void MineField::onChordRequested(std::uint32_t row, std::uint32_t col)
{
    if (m_state != GameState::Playing)
    {
        return;
    }
    auto *cell = m_buttons[row][col];
    if (!cell->isOpened() || cell->Number() == 0)
    {
        return;
    }

    std::uint32_t flagsAround = 0;
    for (int dr = -1; dr <= 1; ++dr)
    {
        for (int dc = -1; dc <= 1; ++dc)
        {
            if (dr == 0 && dc == 0)
            {
                continue;
            }
            const int nr = static_cast<int>(row) + dr;
            const int nc = static_cast<int>(col) + dc;
            if (inBounds(nr, nc, m_difficulty.height, m_difficulty.width) && m_buttons[nr][nc]->isFlagged())
            {
                ++flagsAround;
            }
        }
    }

    if (flagsAround != cell->Number())
    {
        return;
    }

    for (int dr = -1; dr <= 1; ++dr)
    {
        for (int dc = -1; dc <= 1; ++dc)
        {
            if (dr == 0 && dc == 0)
            {
                continue;
            }
            const int nr = static_cast<int>(row) + dr;
            const int nc = static_cast<int>(col) + dc;
            if (inBounds(nr, nc, m_difficulty.height, m_difficulty.width))
            {
                auto *n = m_buttons[nr][nc];
                if (!n->isOpened() && !n->isFlagged())
                {
                    n->Open();
                }
            }
        }
    }
}

void MineField::revealAllMines()
{
    for (std::uint32_t r = 0; r < m_difficulty.height; ++r)
    {
        for (std::uint32_t c = 0; c < m_difficulty.width; ++c)
        {
            auto *btn = m_buttons[r][c];
            if (btn->isMined() && !btn->isOpened())
            {
                btn->revealAsMine();
            }
            else if (!btn->isMined() && btn->isFlagged())
            {
                btn->revealAsWrongFlag();
            }
        }
    }
}

void MineField::flagAllMines()
{
    for (std::uint32_t r = 0; r < m_difficulty.height; ++r)
    {
        for (std::uint32_t c = 0; c < m_difficulty.width; ++c)
        {
            auto *btn = m_buttons[r][c];
            if (btn->isMined() && !btn->isFlagged())
            {
                btn->autoFlag();
            }
        }
    }
}

void MineField::freezeAllCells()
{
    for (auto &row : m_buttons)
    {
        for (auto *btn : row)
        {
            btn->setCellEnabled(false);
        }
    }
}

void MineField::clearAllQuestionMarks()
{
    for (auto &row : m_buttons)
    {
        for (auto *btn : row)
        {
            if (btn)
            {
                btn->clearQuestion();
            }
        }
    }
}

void MineField::setFixedLayout(std::uint32_t width, std::uint32_t height, const std::vector<std::pair<std::uint32_t, std::uint32_t>> &minePositions)
{
    Difficulty diff{width, height, static_cast<std::uint32_t>(minePositions.size())};
    m_difficulty = diff;
    m_state = GameState::Ready;
    m_openedSafeCount = 0;
    m_flagCount = 0;
    m_paused = false;
    m_anyFlagPlaced = false;
    m_minesPlaced = true;

    clearGrid();
    buildGrid();
    if (m_pauseOverlay)
    {
        m_pauseOverlay->hide();
    }

    m_lastMinePositions.clear();
    m_lastMinePositions.reserve(minePositions.size());
    for (const auto &p : minePositions)
    {
        if (p.first < height && p.second < width)
        {
            m_buttons[p.first][p.second]->setMined();
            m_lastMinePositions.emplace_back(p.first, p.second);
        }
    }
    fillNumbers();
    updateMineCountLabel();
    emit mineCountChanged(remainingMines());
}
