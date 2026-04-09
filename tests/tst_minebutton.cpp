#include <QApplication>
#include <QSignalSpy>
#include <QTest>

#include "minefield.h"
#include "minebutton.h"

// Helper: find a non-mined button with Number()==0 (fully empty cell)
static MineButton *findEmptyButton(MineField &field)
{
    for (uint i = 0; i < MineField::getHeight(); ++i)
        for (uint j = 0; j < MineField::getWidth(); ++j)
        {
            MineButton *btn = field.getButton(i, j);
            if (!btn->isMined() && btn->Number() == 0)
                return btn;
        }
    return nullptr;
}

// Helper: find a non-mined button with Number() > 0 (numbered cell)
static MineButton *findNumberedButton(MineField &field)
{
    for (uint i = 0; i < MineField::getHeight(); ++i)
        for (uint j = 0; j < MineField::getWidth(); ++j)
        {
            MineButton *btn = field.getButton(i, j);
            if (!btn->isMined() && btn->Number() > 0)
                return btn;
        }
    return nullptr;
}

// Helper: find a mined button
static MineButton *findMinedButton(MineField &field)
{
    for (uint i = 0; i < MineField::getHeight(); ++i)
        for (uint j = 0; j < MineField::getWidth(); ++j)
        {
            MineButton *btn = field.getButton(i, j);
            if (btn->isMined())
                return btn;
        }
    return nullptr;
}

class TestMineButton : public QObject
{
    Q_OBJECT

private slots:
    void testInitialState();
    void testSetMined();
    void testSetAndGetNumber();
    void testSetNumberIgnoredWhenMined();
    void testPositions();
    void testOpenSetsOpened();
    void testOpenEmptyEmitsCheckNeighbours();
    void testOpenMinedEmitsExplosion();
};

// Freshly constructed buttons are not mined, not opened, not flagged
void TestMineButton::testInitialState()
{
    MineField field;
    for (uint i = 0; i < MineField::getHeight(); ++i)
    {
        for (uint j = 0; j < MineField::getWidth(); ++j)
        {
            MineButton *btn = field.getButton(i, j);
            // isFlagged starts false
            QVERIFY(!btn->isFlagged());
            // isOpened starts false
            QVERIFY(!btn->isOpened());
        }
    }
}

// setMined() flips isMined() from false to true
void TestMineButton::testSetMined()
{
    MineField field;
    // Pick the first non-mined button and mark it
    for (uint i = 0; i < MineField::getHeight(); ++i)
    {
        for (uint j = 0; j < MineField::getWidth(); ++j)
        {
            MineButton *btn = field.getButton(i, j);
            if (!btn->isMined())
            {
                btn->setMined();
                QVERIFY(btn->isMined());
                return;
            }
        }
    }
    QFAIL("No non-mined button found");
}

// setNumber / Number round-trip for values 1–8 on a non-mined cell
void TestMineButton::testSetAndGetNumber()
{
    MineField field;
    MineButton *btn = nullptr;
    for (uint i = 0; i < MineField::getHeight() && !btn; ++i)
        for (uint j = 0; j < MineField::getWidth() && !btn; ++j)
            if (!field.getButton(i, j)->isMined())
                btn = field.getButton(i, j);

    QVERIFY(btn != nullptr);

    for (uint n = 1; n <= 8; ++n)
    {
        btn->setNumber(n);
        QCOMPARE(btn->Number(), n);
    }
}

// setNumber() on a mined button must NOT overwrite the stored number
void TestMineButton::testSetNumberIgnoredWhenMined()
{
    MineField field;
    MineButton *btn = findMinedButton(field);
    if (!btn)
        QSKIP("No mined button available");

    uint before = btn->Number();
    btn->setNumber(5);
    QCOMPARE(btn->Number(), before);
}

// Every button's getX()/getY() must equal its row/column in the grid
void TestMineButton::testPositions()
{
    MineField field;
    for (uint i = 0; i < MineField::getHeight(); ++i)
        for (uint j = 0; j < MineField::getWidth(); ++j)
        {
            MineButton *btn = field.getButton(i, j);
            QCOMPARE(btn->getX(), i);
            QCOMPARE(btn->getY(), j);
        }
}

// Open() marks the button as opened
void TestMineButton::testOpenSetsOpened()
{
    MineField field;
    MineButton *btn = findNumberedButton(field);
    if (!btn)
        QSKIP("No numbered button available");

    QVERIFY(!btn->isOpened());
    btn->Open();
    QVERIFY(btn->isOpened());
}

// Opening a cell with Number()==0 and not mined emits checkNeighbours
void TestMineButton::testOpenEmptyEmitsCheckNeighbours()
{
    MineField field;
    MineButton *btn = findEmptyButton(field);
    if (!btn)
        QSKIP("No empty (number==0, non-mined) button found in this layout");

    QSignalSpy spy(btn, &MineButton::checkNeighbours);
    btn->Open();
    QVERIFY(spy.count() >= 1);
}

// Opening a mined cell emits explosion
void TestMineButton::testOpenMinedEmitsExplosion()
{
    MineField field;
    MineButton *btn = findMinedButton(field);
    if (!btn)
        QSKIP("No mined button available");

    QSignalSpy spy(btn, &MineButton::explosion);
    btn->Open();
    QVERIFY(spy.count() >= 1);
}

QTEST_MAIN(TestMineButton)
#include "tst_minebutton.moc"
