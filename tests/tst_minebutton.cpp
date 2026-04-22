#include "../minebutton.h"

#include <QMouseEvent>
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
    void testOpenEmitsCellPressedFirst();
    void testDoubleOpenIsIdempotent();
    void testFlagToggleEmitsFlagToggled();
    void testFlaggedCellIgnoresLeftClick();
    void testDisabledCellIgnoresPress();
    void testMiddleClickOnOpenedEmitsChord();
    void testMiddleClickOnUnopenedDoesNotEmitChord();
    void testRevealAsMineMarksOpened();
};

static void sendMouseClick(MineButton &btn, Qt::MouseButton button)
{
    QMouseEvent press(QEvent::MouseButtonPress, QPointF(5, 5), QPointF(5, 5), button, button, Qt::NoModifier);
    QCoreApplication::sendEvent(&btn, &press);
}

void TestMineButton::testInitialState()
{
    MineButton btn(0, 0, nullptr);
    QVERIFY(!btn.isMined());
    QVERIFY(!btn.isOpened());
    QVERIFY(!btn.isFlagged());
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
    btn.Open();
    QVERIFY(btn.isOpened());
}

void TestMineButton::testOpenEmitsCheckNeighboursWhenEmpty()
{
    MineButton btn(2, 3, nullptr);
    QSignalSpy spy(&btn, &MineButton::checkNeighbours);
    btn.Open();
    QCOMPARE(spy.count(), 1);
    auto args = spy.takeFirst();
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
    QCOMPARE(btn.Number(), 0u);
}

void TestMineButton::testOpenEmitsCellPressedFirst()
{
    MineButton btn(4, 5, nullptr);
    QSignalSpy pressedSpy(&btn, &MineButton::cellPressed);
    QSignalSpy openedSpy(&btn, &MineButton::cellOpened);
    btn.Open();
    QCOMPARE(pressedSpy.count(), 1);
    QCOMPARE(openedSpy.count(), 1);
}

void TestMineButton::testDoubleOpenIsIdempotent()
{
    MineButton btn(0, 0, nullptr);
    QSignalSpy openedSpy(&btn, &MineButton::cellOpened);
    btn.Open();
    btn.Open();
    btn.Open();
    QCOMPARE(openedSpy.count(), 1);
}

void TestMineButton::testFlagToggleEmitsFlagToggled()
{
    MineButton btn(1, 1, nullptr);
    QSignalSpy spy(&btn, &MineButton::flagToggled);

    sendMouseClick(btn, Qt::RightButton);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(2).toBool(), true);
    QVERIFY(btn.isFlagged());

    sendMouseClick(btn, Qt::RightButton);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(2).toBool(), false);
    QVERIFY(!btn.isFlagged());
}

void TestMineButton::testFlaggedCellIgnoresLeftClick()
{
    MineButton btn(0, 0, nullptr);
    sendMouseClick(btn, Qt::RightButton);
    QVERIFY(btn.isFlagged());

    QSignalSpy openedSpy(&btn, &MineButton::cellOpened);
    sendMouseClick(btn, Qt::LeftButton);
    QCOMPARE(openedSpy.count(), 0);
    QVERIFY(!btn.isOpened());
}

void TestMineButton::testDisabledCellIgnoresPress()
{
    MineButton btn(0, 0, nullptr);
    btn.setCellEnabled(false);
    QSignalSpy openedSpy(&btn, &MineButton::cellOpened);
    sendMouseClick(btn, Qt::LeftButton);
    QCOMPARE(openedSpy.count(), 0);
    QVERIFY(!btn.isOpened());
}

void TestMineButton::testMiddleClickOnOpenedEmitsChord()
{
    MineButton btn(2, 3, nullptr);
    btn.Open();

    QSignalSpy chordSpy(&btn, &MineButton::chordRequested);
    sendMouseClick(btn, Qt::MiddleButton);
    QCOMPARE(chordSpy.count(), 1);
    auto args = chordSpy.takeFirst();
    QCOMPARE(args.at(0).toUInt(), 2u);
    QCOMPARE(args.at(1).toUInt(), 3u);
}

void TestMineButton::testMiddleClickOnUnopenedDoesNotEmitChord()
{
    MineButton btn(0, 0, nullptr);
    QSignalSpy chordSpy(&btn, &MineButton::chordRequested);
    sendMouseClick(btn, Qt::MiddleButton);
    QCOMPARE(chordSpy.count(), 0);
}

void TestMineButton::testRevealAsMineMarksOpened()
{
    MineButton btn(0, 0, nullptr);
    btn.setMined();
    btn.revealAsMine();
    QVERIFY(btn.isOpened());
}

QTEST_MAIN(TestMineButton)
#include "tst_minebutton.moc"
