#include "minefield.h"

#include <QSizePolicy>

#include <algorithm>
#include <random>

namespace
{
bool inBounds(int row, int col, std::uint32_t rows, std::uint32_t cols) { return row >= 0 && row < static_cast<int>(rows) && col >= 0 && col < static_cast<int>(cols); }
} // namespace

MineField::MineField(QWidget *parent) : QWidget{parent}
{
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
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

    clearGrid();
    buildGrid();
    updateMineCountLabel();
    emit mineCountChanged(remainingMines());
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
    adjustSize();
}

void MineField::wireButton(MineButton *button)
{
    connect(button, &MineButton::cellPressed, this, &MineField::onCellPressed);
    connect(button, &MineButton::cellOpened, this, &MineField::onCellOpened);
    connect(button, &MineButton::explosion, this, &MineField::onMineExploded);
    connect(button, &MineButton::checkNeighbours, this, &MineField::onCheckNeighbours);
    connect(button, &MineButton::flagToggled, this, &MineField::onFlagToggled);
    connect(button, &MineButton::chordRequested, this, &MineField::onChordRequested);
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

void MineField::setFixedLayout(std::uint32_t width, std::uint32_t height, const std::vector<std::pair<std::uint32_t, std::uint32_t>> &minePositions)
{
    Difficulty diff{width, height, static_cast<std::uint32_t>(minePositions.size())};
    m_difficulty = diff;
    m_state = GameState::Ready;
    m_openedSafeCount = 0;
    m_flagCount = 0;
    m_minesPlaced = true;

    clearGrid();
    buildGrid();

    for (const auto &p : minePositions)
    {
        if (p.first < height && p.second < width)
        {
            m_buttons[p.first][p.second]->setMined();
        }
    }
    fillNumbers();
    updateMineCountLabel();
    emit mineCountChanged(remainingMines());
}
