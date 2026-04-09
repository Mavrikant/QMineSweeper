#ifndef MINEFIELD_H
#define MINEFIELD_H

#include "minebutton.h"

#include <QGridLayout>
#include <QLabel>
#include <QPointer>
#include <QWidget>

#include <array>
#include <cstdint>

class MineField : public QWidget
{
    Q_OBJECT
  public:
    explicit MineField(QWidget *parent = nullptr);

    void incrementflagCount();
    void getMineCountLabel(QLabel *label);

  public slots:
    void checkNeighbours(std::uint32_t i, std::uint32_t j);

  private:
    void fillMines();
    void fillNumbers();

    static constexpr std::uint32_t Width = 15;
    static constexpr std::uint32_t Height = 15;
    static constexpr int MineCount = 20;

    QGridLayout m_grid{this};
    std::array<std::array<MineButton *, Width>, Height> m_buttons{};
    QPointer<QLabel> m_mineCountLabel;
    int m_flagCount{0};
};

#endif // MINEFIELD_H
