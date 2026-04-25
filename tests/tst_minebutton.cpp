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
    void testLeftPressEmitsPressStart();
    void testRightPressDoesNotEmitPressStart();
    void testMiddlePressEmitsPressStart();
    void testMouseReleaseEmitsPressEnd();
    void testDisabledCellEmitsNoPressSignals();
    void testColorBlindPaletteToggleStatic();
    void testColorBlindPaletteAffectsOpenedNumberStyle();
    void testRefreshNumberStyleIsNoOpOnUnopened();
    void testRefreshNumberStyleIsNoOpOnMined();
    void testRefreshNumberStyleIsNoOpOnZero();
    void cleanup();
};

static void sendMouseClick(MineButton &btn, Qt::MouseButton button)
{
    QMouseEvent press(QEvent::MouseButtonPress, QPointF(5, 5), QPointF(5, 5), button, button, Qt::NoModifier);
    QCoreApplication::sendEvent(&btn, &press);
}

static void sendMouseRelease(MineButton &btn, Qt::MouseButton button)
{
    QMouseEvent release(QEvent::MouseButtonRelease, QPointF(5, 5), QPointF(5, 5), button, Qt::NoButton, Qt::NoModifier);
    QCoreApplication::sendEvent(&btn, &release);
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

void TestMineButton::testLeftPressEmitsPressStart()
{
    MineButton btn(0, 0, nullptr);
    QSignalSpy startSpy(&btn, &MineButton::pressStart);
    sendMouseClick(btn, Qt::LeftButton);
    QCOMPARE(startSpy.count(), 1);
}

void TestMineButton::testRightPressDoesNotEmitPressStart()
{
    // Flag-only clicks must NOT trigger the tension smiley — only cells that
    // could actually be revealed (or chorded) contribute to the hold state.
    MineButton btn(0, 0, nullptr);
    QSignalSpy startSpy(&btn, &MineButton::pressStart);
    sendMouseClick(btn, Qt::RightButton);
    QCOMPARE(startSpy.count(), 0);
}

void TestMineButton::testMiddlePressEmitsPressStart()
{
    MineButton btn(0, 0, nullptr);
    QSignalSpy startSpy(&btn, &MineButton::pressStart);
    sendMouseClick(btn, Qt::MiddleButton);
    QCOMPARE(startSpy.count(), 1);
}

void TestMineButton::testMouseReleaseEmitsPressEnd()
{
    MineButton btn(0, 0, nullptr);
    QSignalSpy endSpy(&btn, &MineButton::pressEnd);
    sendMouseClick(btn, Qt::LeftButton);
    QCOMPARE(endSpy.count(), 0);
    sendMouseRelease(btn, Qt::LeftButton);
    QCOMPARE(endSpy.count(), 1);
}

void TestMineButton::testDisabledCellEmitsNoPressSignals()
{
    // Frozen cells (post-win/loss) must not emit tension signals — otherwise
    // a stray click on a dead board would flip the indicator away from 😎/😵.
    MineButton btn(0, 0, nullptr);
    btn.setCellEnabled(false);
    QSignalSpy startSpy(&btn, &MineButton::pressStart);
    sendMouseClick(btn, Qt::LeftButton);
    QCOMPARE(startSpy.count(), 0);
}

void TestMineButton::testColorBlindPaletteToggleStatic()
{
    // Defaults off; setter flips the app-global bit in both directions.
    QVERIFY(!MineButton::colorBlindPaletteEnabled());
    MineButton::setColorBlindPaletteEnabled(true);
    QVERIFY(MineButton::colorBlindPaletteEnabled());
    MineButton::setColorBlindPaletteEnabled(false);
    QVERIFY(!MineButton::colorBlindPaletteEnabled());
}

void TestMineButton::testColorBlindPaletteAffectsOpenedNumberStyle()
{
    // Open a numbered cell under the classic palette, snapshot its
    // stylesheet, flip the palette, refresh, and confirm the color rule
    // changed. We don't assert on specific RGB values — the palettes are
    // an implementation detail and may be retuned; the contract is "it is
    // different".
    MineButton::setColorBlindPaletteEnabled(false);
    MineButton btn(0, 0, nullptr);
    btn.setNumber(3); // one of the classic-vs-Okabe-Ito divergent digits
    btn.Open();
    const QString classicStyle = btn.styleSheet();
    QVERIFY(classicStyle.contains(QStringLiteral("color: rgb(")));

    MineButton::setColorBlindPaletteEnabled(true);
    btn.refreshNumberStyle();
    const QString cbStyle = btn.styleSheet();
    QVERIFY(cbStyle.contains(QStringLiteral("color: rgb(")));
    QVERIFY(classicStyle != cbStyle);

    // Flipping back restores the classic color rule. Using equality would
    // be fragile against stylesheet whitespace; check that flipping again
    // yields something that differs from the colourblind form.
    MineButton::setColorBlindPaletteEnabled(false);
    btn.refreshNumberStyle();
    QVERIFY(btn.styleSheet() != cbStyle);
}

void TestMineButton::testRefreshNumberStyleIsNoOpOnUnopened()
{
    // An unopened cell has no numbered visual to refresh; refreshing must
    // leave its baseline stylesheet alone (no colour rule injected).
    MineButton::setColorBlindPaletteEnabled(true);
    MineButton btn(0, 0, nullptr);
    btn.setNumber(3);
    const QString before = btn.styleSheet();
    btn.refreshNumberStyle();
    QCOMPARE(btn.styleSheet(), before);
    QVERIFY(!btn.styleSheet().contains(QStringLiteral("color: rgb(")));
}

void TestMineButton::testRefreshNumberStyleIsNoOpOnMined()
{
    // A mined + opened cell shows the explosion style. Refreshing number
    // style must not overwrite the loss-state visual with a number-coloured
    // background.
    MineButton btn(0, 0, nullptr);
    btn.setMined();
    btn.Open(); // paints kMineStyle
    const QString afterExplosion = btn.styleSheet();
    MineButton::setColorBlindPaletteEnabled(true);
    btn.refreshNumberStyle();
    QCOMPARE(btn.styleSheet(), afterExplosion);
}

void TestMineButton::testRefreshNumberStyleIsNoOpOnZero()
{
    // A number=0 opened cell has no visible digit, so the palette has
    // nothing to re-render there. refreshNumberStyle must leave its
    // stylesheet untouched regardless of palette mode — otherwise the
    // opened background would flicker through palette toggles with no
    // user-visible effect.
    MineButton btn(0, 0, nullptr);
    btn.setNumber(0);
    btn.Open();
    const QString before = btn.styleSheet();
    MineButton::setColorBlindPaletteEnabled(true);
    btn.refreshNumberStyle();
    QCOMPARE(btn.styleSheet(), before);
}

void TestMineButton::cleanup()
{
    // Every test leaves the global toggle in a defined state for the next one.
    MineButton::setQuestionMarksEnabled(true);
    MineButton::setColorBlindPaletteEnabled(false);
}

QTEST_MAIN(TestMineButton)
#include "tst_minebutton.moc"
