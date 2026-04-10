#include "minefield.h"

#include "minebutton.h"

#include <QMouseEvent>

#include <random>

MineField::MineField(QWidget *parent) : QWidget{parent}
{
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    for (std::uint32_t i = 0; i < Height; ++i)
    {
        for (std::uint32_t j = 0; j < Width; ++j)
        {
            auto *button = new MineButton(i, j, this);
            button->setContentsMargins(0, 0, 0, 0);
            connect(button, &MineButton::checkNeighbours, this, &MineField::checkNeighbours);
            m_grid.addWidget(button, static_cast<int>(i), static_cast<int>(j));
            m_buttons[i][j] = button;
        }
    }
    m_grid.setSpacing(0);
    m_grid.setContentsMargins(0, 0, 0, 0);
    fillMines();
    fillNumbers();
}

void MineField::incrementflagCount()
{
    ++m_flagCount;
    if (m_mineCountLabel)
    {
        m_mineCountLabel->setNum(MineCount - m_flagCount);
    }
}

void MineField::getMineCountLabel(QLabel *label)
{
    m_mineCountLabel = label;
    if (m_mineCountLabel)
    {
        m_mineCountLabel->setNum(MineCount - m_flagCount);
    }
}

void MineField::fillMines()
{
    std::random_device rd;
    std::mt19937 gen{rd()};
    std::uniform_int_distribution<std::uint32_t> distHeight{0, Height - 1};
    std::uniform_int_distribution<std::uint32_t> distWidth{0, Width - 1};

    int placed = 0;
    while (placed < MineCount)
    {
        const std::uint32_t h = distHeight(gen);
        const std::uint32_t w = distWidth(gen);
        if (!m_buttons[h][w]->isMined())
        {
            m_buttons[h][w]->setMined();
            ++placed;
        }
    }
}

void MineField::fillNumbers()
{
    for (std::uint32_t i = 0; i < Height; ++i)
    {
        for (std::uint32_t j = 0; j < Width; ++j)
        {
            std::uint32_t mineNumber = 0;

            for (int a = -1; a <= 1; ++a)
            {
                for (int b = -1; b <= 1; ++b)
                {
                    const int ni = static_cast<int>(i) + a;
                    const int nj = static_cast<int>(j) + b;
                    if (ni >= 0 && ni < static_cast<int>(Height) && nj >= 0 && nj < static_cast<int>(Width))
                    {
                        mineNumber += m_buttons[ni][nj]->isMined() ? 1u : 0u;
                    }
                }
            }
            m_buttons[i][j]->setNumber(mineNumber);
        }
    }
}

void MineField::checkNeighbours(std::uint32_t i, std::uint32_t j)
{
    for (int a = -1; a <= 1; ++a)
    {
        for (int b = -1; b <= 1; ++b)
        {
            const int ni = static_cast<int>(i) + a;
            const int nj = static_cast<int>(j) + b;
            if (ni >= 0 && ni < static_cast<int>(Height) && nj >= 0 && nj < static_cast<int>(Width))
            {
                if (!m_buttons[ni][nj]->isOpened())
                {
                    m_buttons[ni][nj]->Open();
                }
            }
        }
    }
    update();
}
