#include "../minebutton.h"

#include <QSignalSpy>
#include <QtTest>

class TestMineButton : public QObject
{
    Q_OBJECT

  private slots:
    void testInitialState();
    void testSetMinedAndIsMined();
    void testSetNumberAndNumber();
    void testIsOpenedInitiallyFalse();
    void testOpenSetsOpened();
    void testOpenEmitsCheckNeighboursWhenEmpty();
    void testOpenEmitsExplosionWhenMined();
    void testSetNumberDoesNotOverrideWhenMined();
};

void TestMineButton::testInitialState()
{
    MineButton btn(0, 0, nullptr);
    QVERIFY(!btn.isMined());
    QVERIFY(!btn.isOpened());
    QCOMPARE(btn.Number(), 0u);
}

void TestMineButton::testSetMinedAndIsMined()
{
    MineButton btn(1, 2, nullptr);
    QVERIFY(!btn.isMined());
    btn.setMined();
    QVERIFY(btn.isMined());
}

void TestMineButton::testSetNumberAndNumber()
{
    MineButton btn(0, 0, nullptr);
    btn.setNumber(3);
    QCOMPARE(btn.Number(), 3u);
}

void TestMineButton::testIsOpenedInitiallyFalse()
{
    MineButton btn(0, 0, nullptr);
    QVERIFY(!btn.isOpened());
}

void TestMineButton::testOpenSetsOpened()
{
    MineButton btn(0, 0, nullptr);
    QVERIFY(!btn.isOpened());
    btn.Open();
    QVERIFY(btn.isOpened());
}

void TestMineButton::testOpenEmitsCheckNeighboursWhenEmpty()
{
    MineButton btn(2, 3, nullptr);
    // number == 0 and not mined -> checkNeighbours signal
    QSignalSpy spy(&btn, &MineButton::checkNeighbours);
    btn.Open();
    QCOMPARE(spy.count(), 1);
    QList<QVariant> args = spy.takeFirst();
    QCOMPARE(args.at(0).toUInt(), 2u);
    QCOMPARE(args.at(1).toUInt(), 3u);
}

void TestMineButton::testOpenEmitsExplosionWhenMined()
{
    MineButton btn(0, 0, nullptr);
    btn.setMined();
    QSignalSpy spy(&btn, &MineButton::explosion);
    btn.Open();
    QCOMPARE(spy.count(), 1);
}

void TestMineButton::testSetNumberDoesNotOverrideWhenMined()
{
    MineButton btn(0, 0, nullptr);
    btn.setMined();
    btn.setNumber(5);
    // When mined, setNumber should not change the stored number
    QCOMPARE(btn.Number(), 0u);
}

QTEST_MAIN(TestMineButton)
#include "tst_minebutton.moc"
