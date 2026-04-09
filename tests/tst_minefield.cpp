#include <QApplication>
#include <QTest>

#include "minefield.h"
#include "minebutton.h"

class TestMineField : public QObject
{
    Q_OBJECT

private slots:
    void testDimensions();
    void testMineCountBounds();
    void testFillNumbersConsistency();
    void testInitialFlagCount();
    void testIncrementFlagCount();
    void testGetButtonNonNull();
};

// Grid constants match the game definition
void TestMineField::testDimensions()
{
    QCOMPARE(MineField::getWidth(), 15u);
    QCOMPARE(MineField::getHeight(), 15u);
    QCOMPARE(MineField::getMineCount(), 20);
}

// fillMines() uses rand() with no duplicate-prevention, so the actual mined
// count may be < MineCount when placements collide. At least 1 mine must exist.
void TestMineField::testMineCountBounds()
{
    MineField field;

    int mined = 0;
    for (uint i = 0; i < MineField::getHeight(); ++i)
        for (uint j = 0; j < MineField::getWidth(); ++j)
            if (field.getButton(i, j)->isMined())
                ++mined;

    QVERIFY2(mined >= 1, "Expected at least 1 mine");
    QVERIFY2(mined <= MineField::getMineCount(), "More mines than MineCount");
}

// For every non-mined cell, Number() must equal the count of mined neighbours
void TestMineField::testFillNumbersConsistency()
{
    MineField field;

    const int H = static_cast<int>(MineField::getHeight());
    const int W = static_cast<int>(MineField::getWidth());

    for (int i = 0; i < H; ++i)
    {
        for (int j = 0; j < W; ++j)
        {
            MineButton *btn = field.getButton(static_cast<uint>(i), static_cast<uint>(j));
            if (btn->isMined())
                continue;

            uint expected = 0;
            for (int a = -1; a <= 1; ++a)
                for (int b = -1; b <= 1; ++b)
                    if (i + a >= 0 && i + a < H && j + b >= 0 && j + b < W)
                        expected += field.getButton(static_cast<uint>(i + a), static_cast<uint>(j + b))->isMined() ? 1u : 0u;

            QCOMPARE(btn->Number(), expected);
        }
    }
}

// Flag count starts at zero
void TestMineField::testInitialFlagCount()
{
    MineField field;
    QCOMPARE(field.getFlagCount(), 0);
}

// incrementflagCount() increments the internal counter
void TestMineField::testIncrementFlagCount()
{
    MineField field;
    // Wire up the label that incrementflagCount() writes to
    QLabel label;
    field.getMineCountLabel(&label);

    field.incrementflagCount();
    QCOMPARE(field.getFlagCount(), 1);

    field.incrementflagCount();
    QCOMPARE(field.getFlagCount(), 2);
}

// getButton() must return a valid pointer for every cell in the grid
void TestMineField::testGetButtonNonNull()
{
    MineField field;
    for (uint i = 0; i < MineField::getHeight(); ++i)
        for (uint j = 0; j < MineField::getWidth(); ++j)
            QVERIFY(field.getButton(i, j) != nullptr);
}

QTEST_MAIN(TestMineField)
#include "tst_minefield.moc"
