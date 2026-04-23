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
    void testRightClickCyclesNoneFlagQuestionNone();
    void testQuestionDoesNotBlockOpen();
    void testQuestionToNoneDoesNotEmitFlagToggled();
    void testLeftClickBlockedByFlagNotByQuestion();
    void testQuestionMarksDisabledSkipsQuestion();
    void testClearQuestionResetsMarker();
    void cleanup();
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

void TestMineButton::testRightClickCyclesNoneFlagQuestionNone()
{
    MineButton btn(0, 0, nullptr);
    QCOMPARE(btn.marker(), CellMarker::None);
    QSignalSpy spy(&btn, &MineButton::flagToggled);

    sendMouseClick(btn, Qt::RightButton); // → Flag
    QCOMPARE(btn.marker(), CellMarker::Flag);
    QVERIFY(btn.isFlagged());
    QVERIFY(!btn.isQuestion());
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(2).toBool(), true);

    sendMouseClick(btn, Qt::RightButton); // → Question
    QCOMPARE(btn.marker(), CellMarker::Question);
    QVERIFY(!btn.isFlagged());
    QVERIFY(btn.isQuestion());
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(2).toBool(), false);

    sendMouseClick(btn, Qt::RightButton); // → None
    QCOMPARE(btn.marker(), CellMarker::None);
    QVERIFY(!btn.isFlagged());
    QVERIFY(!btn.isQuestion());
}

void TestMineButton::testQuestionDoesNotBlockOpen()
{
    MineButton btn(0, 0, nullptr);
    sendMouseClick(btn, Qt::RightButton); // Flag
    sendMouseClick(btn, Qt::RightButton); // Question
    QCOMPARE(btn.marker(), CellMarker::Question);

    QSignalSpy openedSpy(&btn, &MineButton::cellOpened);
    sendMouseClick(btn, Qt::LeftButton);
    QCOMPARE(openedSpy.count(), 1);
    QVERIFY(btn.isOpened());
    QCOMPARE(btn.marker(), CellMarker::None); // cleared on reveal
}

void TestMineButton::testQuestionToNoneDoesNotEmitFlagToggled()
{
    MineButton btn(0, 0, nullptr);
    sendMouseClick(btn, Qt::RightButton); // None → Flag  (1)
    sendMouseClick(btn, Qt::RightButton); // Flag → Question  (2)
    QSignalSpy spy(&btn, &MineButton::flagToggled);
    sendMouseClick(btn, Qt::RightButton); // Question → None  (silent)
    QCOMPARE(spy.count(), 0);
}

void TestMineButton::testLeftClickBlockedByFlagNotByQuestion()
{
    MineButton a(0, 0, nullptr);
    sendMouseClick(a, Qt::RightButton); // Flag
    QSignalSpy openedA(&a, &MineButton::cellOpened);
    sendMouseClick(a, Qt::LeftButton);
    QCOMPARE(openedA.count(), 0);

    MineButton b(0, 0, nullptr);
    sendMouseClick(b, Qt::RightButton);
    sendMouseClick(b, Qt::RightButton); // Question
    QSignalSpy openedB(&b, &MineButton::cellOpened);
    sendMouseClick(b, Qt::LeftButton);
    QCOMPARE(openedB.count(), 1);
}

void TestMineButton::testQuestionMarksDisabledSkipsQuestion()
{
    MineButton::setQuestionMarksEnabled(false);
    MineButton btn(0, 0, nullptr);
    QSignalSpy spy(&btn, &MineButton::flagToggled);

    sendMouseClick(btn, Qt::RightButton); // None → Flag
    QCOMPARE(btn.marker(), CellMarker::Flag);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(2).toBool(), true);

    sendMouseClick(btn, Qt::RightButton); // Flag → None (skip Question)
    QCOMPARE(btn.marker(), CellMarker::None);
    QVERIFY(!btn.isQuestion());
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(2).toBool(), false);

    sendMouseClick(btn, Qt::RightButton); // None → Flag again
    QCOMPARE(btn.marker(), CellMarker::Flag);
}

void TestMineButton::testClearQuestionResetsMarker()
{
    MineButton::setQuestionMarksEnabled(true);
    MineButton btn(0, 0, nullptr);
    sendMouseClick(btn, Qt::RightButton); // Flag
    sendMouseClick(btn, Qt::RightButton); // Question
    QCOMPARE(btn.marker(), CellMarker::Question);

    QSignalSpy spy(&btn, &MineButton::flagToggled);
    btn.clearQuestion();
    QCOMPARE(btn.marker(), CellMarker::None);
    // No flag transition — clearQuestion should not emit flagToggled.
    QCOMPARE(spy.count(), 0);

    // A cleared cell stays openable.
    QSignalSpy openedSpy(&btn, &MineButton::cellOpened);
    sendMouseClick(btn, Qt::LeftButton);
    QCOMPARE(openedSpy.count(), 1);
}

void TestMineButton::cleanup()
{
    // Every test leaves the global toggle in a defined state for the next one.
    MineButton::setQuestionMarksEnabled(true);
}

QTEST_MAIN(TestMineButton)
#include "tst_minebutton.moc"
