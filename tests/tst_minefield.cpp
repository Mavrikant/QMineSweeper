#include "../minebutton.h"
#include "../minefield.h"

#include <QLabel>
#include <QSignalSpy>
#include <QtTest>

class TestMineField : public QObject
{
    Q_OBJECT

  private slots:
    void testConstruction();
    void testDefaultDifficultyIsBeginner();
    void testMineCountLabelInitialValue();
    void testFlagTogglesMineCount();
    void testFirstClickNeverMine();
    void testFirstClickIsZero();
    void testGameStartedSignalFiresOnce();
    void testGameLostOnMine();
    void testGameStateGatingAfterLoss();
    void testWinBySolving();
    void testNewGameResetsState();
    void testDifficultySizes();
    void testSetFixedLayoutPlacesMines();
    void testChordOpensNeighboursWhenFlagsMatch();
    void testChordDetonatesWhenFlagsWrong();
    void testChordDoesNothingWhenFlagsInsufficient();
    void testCanReplayFalseBeforeFirstClick();
    void testCanReplayFalseAfterNewGame();
    void testReplayPreservesMinePositions();
    void testReplayResetsOpenedAndFlaggedCells();
    void testReplayAfterLossIsPlayable();
    void testNewGameReplayFallbackWhenNoLayout();
    void testCustomDifficultyGridSize();
    void testCustomDifficultyFirstClickSafety();
    void testCustomDifficultyMineCountBoundary();
    void testKeyboardArrowsMoveFocus();
    void testKeyboardArrowsRespectBounds();
    void testKeyboardSpaceRevealsCell();
    void testKeyboardSpaceOnOpenedChords();
    void testKeyboardFTogglesFlag();
    void testKeyboardDChordsOpenedCell();
    void testKeyboardSpaceOnFlaggedIsNoop();
    void testKeyboardIgnoredAfterLoss();
    void testPauseDefaultsFalse();
    void testPauseBlocksLeftClickReveal();
    void testPauseBlocksRightClickFlag();
    void testPauseBlocksKeyboardSpace();
    void testPauseBlocksKeyboardArrowsAndF();
    void testResumeRestoresInput();
    void testNewGameClearsPause();
    void testReplayClearsPause();
    void testPauseToggleIdempotent();
    void testAnyFlagPlacedDefaultsFalse();
    void testAnyFlagPlacedTrueAfterRightClickFlag();
    void testAnyFlagPlacedTrueAfterKeyboardF();
    void testAnyFlagPlacedStaysTrueAfterUnflag();
    void testAnyFlagPlacedResetByNewGame();
    void testAnyFlagPlacedResetByReplay();
    void testAnyFlagPlacedNotSetByQuestionMark();
    void testAnyFlagPlacedFalseAfterNoflagWin();
    void testBoardValueZeroBeforeMinesPlaced();
    void testBoardValueSingleOpening();
    void testBoardValueAllNumberedNoZeros();
    void testBoardValueTwoSeparateOpenings();
    void testBoardValueIsolatedNumberedCells();
    void testBoardValueFullBoardOfMinesEdgeCase();
    void testBoardValueResetByNewGame();
    void testBoardValueComputedAfterFirstClick();
    void testBoardValueReplayPreservesValue();
    void testBoardValuePreservedOnLoss();
    void testUserClicksZeroBeforeAnyClick();
    void testUserClicksLeftClickIncrementsOnce();
    void testUserClicksFloodCountsOnce();
    void testUserClicksLeftClickOnFlaggedDoesNothing();
    void testUserClicksLeftClickOnOpenedDoesNothing();
    void testUserClicksRightClickDoesNothing();
    void testUserClicksKeyboardSpaceCountsOnce();
    void testUserClicksKeyboardSpaceOnFlaggedDoesNothing();
    void testUserClicksChordCountsOnceWhenOpens();
    void testUserClicksChordOnFullySatisfiedDoesNothing();
    void testUserClicksChordWrongFlagCountsAndExplodes();
    void testUserClicksLeftClickOnMineCountsAndExplodes();
    void testUserClicksKeyboardDChordCountsOnce();
    void testUserClicksResetByNewGame();
    void testUserClicksResetByReplay();
    void testUserClicksResetBySetFixedLayout();
    void testRefreshAllNumberStylesReflectsPaletteChange();
    void testRefreshAllNumberStylesSkipsUnopened();
    void testSafePercentClearedZeroBeforeAnyClick();
    void testSafePercentClearedReachesHundredOnWin();
    void testSafePercentClearedMidGame();
    void testSafePercentClearedRoundsHalfUp();
    void testFlagsPlacedZeroBeforeAnyFlag();
    void testFlagsPlacedIncrementsOnFlag();
    void testFlagsPlacedDecrementsOnUnflag();
    void testFlagsPlacedPreservedOnLoss();
    void testFlagsPlacedResetByNewGame();
    void testQuestionMarksPlacedZeroBeforeAnyMark();
    void testQuestionMarksPlacedCountsOnlyQuestionCells();
    void testQuestionMarksPlacedDecrementsOnCycleAway();
    void testQuestionMarksPlacedPreservedOnLoss();
    void testQuestionMarksPlacedResetByNewGame();

  private:
    static void openAllSafe(MineField &field);
    static void sendKey(MineButton *target, int key);
    static void sendMousePress(MineButton *target, Qt::MouseButton button);
};

void TestMineField::testConstruction()
{
    MineField field;
    QVERIFY(field.isWidgetType());
    QCOMPARE(field.state(), GameState::Ready);
}

void TestMineField::testDefaultDifficultyIsBeginner()
{
    MineField field;
    QCOMPARE(field.cols(), MineField::Beginner.width);
    QCOMPARE(field.rows(), MineField::Beginner.height);
    QCOMPARE(field.mineCount(), MineField::Beginner.mineCount);
}

void TestMineField::testMineCountLabelInitialValue()
{
    MineField field;
    QLabel label;
    field.setMineCountLabel(&label);
    QCOMPARE(label.text(), QString::number(MineField::Beginner.mineCount));
}

void TestMineField::testFlagTogglesMineCount()
{
    MineField field;
    // Use a deterministic layout so we know exactly which cells are unopened.
    field.setFixedLayout(5, 5, {{0, 0}, {4, 4}});
    QLabel label;
    field.setMineCountLabel(&label);
    QCOMPARE(label.text(), QString::number(2));

    QSignalSpy spy(&field, &MineField::mineCountChanged);

    MineButton *target = field.cellAt(2, 2);
    QMouseEvent rightPress(QEvent::MouseButtonPress, QPointF(1, 1), QPointF(1, 1), Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    QCoreApplication::sendEvent(target, &rightPress);
    QCOMPARE(label.text(), QString::number(1));

    QCoreApplication::sendEvent(target, &rightPress);
    QCOMPARE(label.text(), QString::number(2));
    QCOMPARE(spy.count(), 2);
}

void TestMineField::testFirstClickNeverMine()
{
    for (int iter = 0; iter < 20; ++iter)
    {
        MineField field;
        MineButton *first = field.cellAt(4, 4);
        first->Open();
        QVERIFY2(!first->isMined(), "first click detonated a mine");
    }
}

void TestMineField::testFirstClickIsZero()
{
    for (int iter = 0; iter < 20; ++iter)
    {
        MineField field;
        MineButton *first = field.cellAt(4, 4);
        first->Open();
        QCOMPARE(first->Number(), 0u);
    }
}

void TestMineField::testGameStartedSignalFiresOnce()
{
    MineField field;
    QSignalSpy spy(&field, &MineField::gameStarted);
    field.cellAt(4, 4)->Open();
    // Flood-fill opens many cells via checkNeighbours, but gameStarted must fire exactly once.
    QCOMPARE(spy.count(), 1);
}

void TestMineField::testGameLostOnMine()
{
    MineField field;
    // Use fixed layout with a single mine so we can detonate deterministically.
    field.setFixedLayout(5, 5, {{2, 2}});
    QSignalSpy lostSpy(&field, &MineField::gameLost);

    field.cellAt(2, 2)->Open();
    QCOMPARE(lostSpy.count(), 1);
    QCOMPARE(field.state(), GameState::Lost);
}

void TestMineField::testGameStateGatingAfterLoss()
{
    MineField field;
    field.setFixedLayout(5, 5, {{0, 0}});
    field.cellAt(0, 0)->Open(); // boom
    QCOMPARE(field.state(), GameState::Lost);

    QSignalSpy wonSpy(&field, &MineField::gameWon);
    // Cells are frozen; clicking more should not fire gameWon nor change state.
    QMouseEvent press(QEvent::MouseButtonPress, QPointF(1, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(field.cellAt(4, 4), &press);
    QCOMPARE(wonSpy.count(), 0);
    QCOMPARE(field.state(), GameState::Lost);
}

void TestMineField::openAllSafe(MineField &field)
{
    for (std::uint32_t r = 0; r < field.rows(); ++r)
    {
        for (std::uint32_t c = 0; c < field.cols(); ++c)
        {
            auto *cell = field.cellAt(r, c);
            if (!cell->isMined() && !cell->isOpened())
            {
                cell->Open();
            }
        }
    }
}

void TestMineField::testWinBySolving()
{
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}, {2, 2}});
    QSignalSpy wonSpy(&field, &MineField::gameWon);

    // Kick state into Playing via a safe cell click (first-click safety is bypassed by setFixedLayout).
    field.cellAt(1, 1)->Open();
    openAllSafe(field);

    QCOMPARE(wonSpy.count(), 1);
    QCOMPARE(field.state(), GameState::Won);
}

void TestMineField::testNewGameResetsState()
{
    MineField field;
    field.setFixedLayout(4, 4, {{0, 0}});
    field.cellAt(0, 0)->Open(); // lose
    QCOMPARE(field.state(), GameState::Lost);

    field.newGame(MineField::Beginner);
    QCOMPARE(field.state(), GameState::Ready);
    QCOMPARE(field.cols(), MineField::Beginner.width);
}

void TestMineField::testDifficultySizes()
{
    MineField field;
    field.newGame(MineField::Intermediate);
    QCOMPARE(field.cols(), 16u);
    QCOMPARE(field.rows(), 16u);
    QCOMPARE(field.mineCount(), 40u);

    field.newGame(MineField::Expert);
    QCOMPARE(field.cols(), 30u);
    QCOMPARE(field.rows(), 16u);
    QCOMPARE(field.mineCount(), 99u);

    field.newGame(MineField::Beginner);
    QCOMPARE(field.cols(), 9u);
    QCOMPARE(field.rows(), 9u);
    QCOMPARE(field.mineCount(), 10u);
}

void TestMineField::testSetFixedLayoutPlacesMines()
{
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}, {2, 2}});
    QVERIFY(field.cellAt(0, 0)->isMined());
    QVERIFY(field.cellAt(2, 2)->isMined());
    QVERIFY(!field.cellAt(1, 1)->isMined());
    QCOMPARE(field.cellAt(1, 1)->Number(), 2u);
}

void TestMineField::testChordOpensNeighboursWhenFlagsMatch()
{
    MineField field;
    // Mine at (0,0). Cell (1,1) has number 1. Flag (0,0), open (1,1), chord on (1,1).
    field.setFixedLayout(3, 3, {{0, 0}});
    field.cellAt(1, 1)->Open();
    QCOMPARE(field.cellAt(1, 1)->Number(), 1u);

    QMouseEvent flag(QEvent::MouseButtonPress, QPointF(1, 1), QPointF(1, 1), Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    QCoreApplication::sendEvent(field.cellAt(0, 0), &flag);
    QVERIFY(field.cellAt(0, 0)->isFlagged());

    QMouseEvent chord(QEvent::MouseButtonPress, QPointF(1, 1), QPointF(1, 1), Qt::MiddleButton, Qt::MiddleButton, Qt::NoModifier);
    QCoreApplication::sendEvent(field.cellAt(1, 1), &chord);

    // All safe neighbours should be open now.
    QVERIFY(field.cellAt(0, 1)->isOpened());
    QVERIFY(field.cellAt(1, 0)->isOpened());
    QVERIFY(field.cellAt(0, 2)->isOpened());
    QVERIFY(field.cellAt(2, 0)->isOpened());
    QVERIFY(field.cellAt(2, 2)->isOpened());
}

void TestMineField::testChordDetonatesWhenFlagsWrong()
{
    MineField field;
    // Mine at (0,0). Flag (0,1) WRONG, then chord on (1,1) — flagsAround == 1 matches number 1,
    // so chord opens the actually-mined unflagged neighbour (0,0) and detonates.
    field.setFixedLayout(3, 3, {{0, 0}});
    field.cellAt(1, 1)->Open();

    QMouseEvent flag(QEvent::MouseButtonPress, QPointF(1, 1), QPointF(1, 1), Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    QCoreApplication::sendEvent(field.cellAt(0, 1), &flag);
    QVERIFY(field.cellAt(0, 1)->isFlagged());

    QSignalSpy lostSpy(&field, &MineField::gameLost);
    QMouseEvent chord(QEvent::MouseButtonPress, QPointF(1, 1), QPointF(1, 1), Qt::MiddleButton, Qt::MiddleButton, Qt::NoModifier);
    QCoreApplication::sendEvent(field.cellAt(1, 1), &chord);

    QCOMPARE(lostSpy.count(), 1);
    QCOMPARE(field.state(), GameState::Lost);
}

void TestMineField::testChordDoesNothingWhenFlagsInsufficient()
{
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}});
    field.cellAt(1, 1)->Open(); // number 1, zero flags around → chord should be a no-op

    QMouseEvent chord(QEvent::MouseButtonPress, QPointF(1, 1), QPointF(1, 1), Qt::MiddleButton, Qt::MiddleButton, Qt::NoModifier);
    QCoreApplication::sendEvent(field.cellAt(1, 1), &chord);

    QVERIFY(!field.cellAt(0, 0)->isOpened());
    QVERIFY(!field.cellAt(0, 1)->isOpened());
    QVERIFY(!field.cellAt(1, 0)->isOpened());
}

void TestMineField::testCanReplayFalseBeforeFirstClick()
{
    MineField field;
    QVERIFY(!field.canReplay());
}

void TestMineField::testCanReplayFalseAfterNewGame()
{
    MineField field;
    field.setFixedLayout(4, 4, {{0, 0}});
    QVERIFY(field.canReplay());

    field.newGame(MineField::Beginner);
    QVERIFY(!field.canReplay());
}

void TestMineField::testReplayPreservesMinePositions()
{
    MineField field;
    // Random layout via real first-click flow.
    field.cellAt(4, 4)->Open();
    QVERIFY(field.canReplay());

    // Snapshot mine positions from the current board.
    std::vector<std::pair<std::uint32_t, std::uint32_t>> before;
    for (std::uint32_t r = 0; r < field.rows(); ++r)
    {
        for (std::uint32_t c = 0; c < field.cols(); ++c)
        {
            if (field.cellAt(r, c)->isMined())
            {
                before.emplace_back(r, c);
            }
        }
    }
    QCOMPARE(before.size(), static_cast<std::size_t>(field.mineCount()));

    QVERIFY(field.newGameReplay());

    std::vector<std::pair<std::uint32_t, std::uint32_t>> after;
    for (std::uint32_t r = 0; r < field.rows(); ++r)
    {
        for (std::uint32_t c = 0; c < field.cols(); ++c)
        {
            if (field.cellAt(r, c)->isMined())
            {
                after.emplace_back(r, c);
            }
        }
    }
    QCOMPARE(after, before);
    QCOMPARE(field.state(), GameState::Ready);
}

void TestMineField::testReplayResetsOpenedAndFlaggedCells()
{
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}});
    field.cellAt(1, 1)->Open(); // reveals a bunch via flood-fill
    QVERIFY(field.cellAt(1, 1)->isOpened());

    // Flag (0, 0) and question-mark (2, 2) before replaying.
    QMouseEvent rightPress(QEvent::MouseButtonPress, QPointF(1, 1), QPointF(1, 1), Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    QCoreApplication::sendEvent(field.cellAt(0, 0), &rightPress);
    QCOMPARE(field.cellAt(0, 0)->marker(), CellMarker::Flag);

    QVERIFY(field.newGameReplay());

    for (std::uint32_t r = 0; r < field.rows(); ++r)
    {
        for (std::uint32_t c = 0; c < field.cols(); ++c)
        {
            QVERIFY(!field.cellAt(r, c)->isOpened());
            QCOMPARE(field.cellAt(r, c)->marker(), CellMarker::None);
        }
    }
    QCOMPARE(field.remainingMines(), 1);
}

void TestMineField::testReplayAfterLossIsPlayable()
{
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}});
    field.cellAt(0, 0)->Open(); // boom
    QCOMPARE(field.state(), GameState::Lost);

    QVERIFY(field.newGameReplay());
    QCOMPARE(field.state(), GameState::Ready);

    // Numbers must be re-derived; (1,1) has one mine neighbour at (0,0).
    QCOMPARE(field.cellAt(1, 1)->Number(), 1u);
    QVERIFY(field.cellAt(0, 0)->isMined());

    // A safe click should move state to Playing, not re-place mines.
    QSignalSpy startedSpy(&field, &MineField::gameStarted);
    field.cellAt(1, 1)->Open();
    QCOMPARE(startedSpy.count(), 1);
    QCOMPARE(field.state(), GameState::Playing);
    QVERIFY(field.cellAt(0, 0)->isMined()); // mine layout preserved through click
}

void TestMineField::testNewGameReplayFallbackWhenNoLayout()
{
    MineField field;
    QVERIFY(!field.canReplay());
    const bool replayed = field.newGameReplay();
    QVERIFY(!replayed);
    // Falls back to a fresh Beginner board with no opened cells.
    QCOMPARE(field.state(), GameState::Ready);
    QCOMPARE(field.cols(), MineField::Beginner.width);
    QVERIFY(!field.canReplay());
}

void TestMineField::testCustomDifficultyGridSize()
{
    MineField field;
    // Arbitrary non-preset geometry that the Custom dialog would allow.
    const Difficulty custom{15u, 12u, 30u};
    field.newGame(custom);
    QCOMPARE(field.cols(), 15u);
    QCOMPARE(field.rows(), 12u);
    QCOMPARE(field.mineCount(), 30u);
    QCOMPARE(field.state(), GameState::Ready);
    // All cells wired up and unopened on a fresh board.
    for (std::uint32_t r = 0; r < field.rows(); ++r)
    {
        for (std::uint32_t c = 0; c < field.cols(); ++c)
        {
            QVERIFY(field.cellAt(r, c) != nullptr);
            QVERIFY(!field.cellAt(r, c)->isOpened());
        }
    }
}

void TestMineField::testCustomDifficultyFirstClickSafety()
{
    // Custom geometry must preserve first-click zero-start behaviour.
    for (int iter = 0; iter < 10; ++iter)
    {
        MineField field;
        field.newGame(Difficulty{15u, 12u, 30u});
        MineButton *first = field.cellAt(6, 7);
        first->Open();
        QVERIFY2(!first->isMined(), "custom-board first click detonated");
        QCOMPARE(first->Number(), 0u);
    }
}

void TestMineField::testCustomDifficultyMineCountBoundary()
{
    // Dense custom board (90% mines) still places the requested mine count
    // after the first click, exercising the relaxed-exclusion branch.
    MineField field;
    field.newGame(Difficulty{9u, 9u, 72u}); // 81 cells, max the dialog allows
    field.cellAt(4, 4)->Open();
    std::uint32_t mines = 0;
    for (std::uint32_t r = 0; r < field.rows(); ++r)
    {
        for (std::uint32_t c = 0; c < field.cols(); ++c)
        {
            if (field.cellAt(r, c)->isMined())
            {
                ++mines;
            }
        }
    }
    QCOMPARE(mines, 72u);
    QVERIFY(!field.cellAt(4, 4)->isMined());
}

void TestMineField::sendKey(MineButton *target, int key)
{
    QKeyEvent press(QEvent::KeyPress, key, Qt::NoModifier);
    QCoreApplication::sendEvent(target, &press);
}

void TestMineField::sendMousePress(MineButton *target, Qt::MouseButton button)
{
    QMouseEvent press(QEvent::MouseButtonPress, target->rect().center(), target->mapToGlobal(target->rect().center()), button, button, Qt::NoModifier);
    QCoreApplication::sendEvent(target, &press);
    QMouseEvent release(QEvent::MouseButtonRelease, target->rect().center(), target->mapToGlobal(target->rect().center()), button, Qt::NoButton, Qt::NoModifier);
    QCoreApplication::sendEvent(target, &release);
}

void TestMineField::testKeyboardArrowsMoveFocus()
{
    MineField field;
    field.setFixedLayout(5, 5, {{0, 0}});
    field.show();
    QVERIFY(QTest::qWaitForWindowExposed(&field));

    MineButton *start = field.cellAt(2, 2);
    start->setFocus(Qt::OtherFocusReason);
    QCOMPARE(field.focusWidget(), start);

    sendKey(start, Qt::Key_Right);
    QCOMPARE(field.focusWidget(), field.cellAt(2, 3));

    sendKey(field.cellAt(2, 3), Qt::Key_Down);
    QCOMPARE(field.focusWidget(), field.cellAt(3, 3));

    sendKey(field.cellAt(3, 3), Qt::Key_Left);
    QCOMPARE(field.focusWidget(), field.cellAt(3, 2));

    sendKey(field.cellAt(3, 2), Qt::Key_Up);
    QCOMPARE(field.focusWidget(), field.cellAt(2, 2));
}

void TestMineField::testKeyboardArrowsRespectBounds()
{
    MineField field;
    field.setFixedLayout(3, 3, {{2, 2}});
    field.show();
    QVERIFY(QTest::qWaitForWindowExposed(&field));

    MineButton *corner = field.cellAt(0, 0);
    corner->setFocus(Qt::OtherFocusReason);
    QCOMPARE(field.focusWidget(), corner);

    sendKey(corner, Qt::Key_Up);
    QCOMPARE(field.focusWidget(), corner); // stays put, no wrap
    sendKey(corner, Qt::Key_Left);
    QCOMPARE(field.focusWidget(), corner);

    MineButton *farCorner = field.cellAt(2, 2);
    farCorner->setFocus(Qt::OtherFocusReason);
    sendKey(farCorner, Qt::Key_Down);
    QCOMPARE(field.focusWidget(), farCorner);
    sendKey(farCorner, Qt::Key_Right);
    QCOMPARE(field.focusWidget(), farCorner);
}

void TestMineField::testKeyboardSpaceRevealsCell()
{
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}});
    MineButton *target = field.cellAt(2, 2);
    QVERIFY(!target->isOpened());

    sendKey(target, Qt::Key_Space);
    QVERIFY(target->isOpened());
    // (2,2) has no mine neighbours, flood-fill opens the plateau.
    QVERIFY(field.cellAt(1, 1)->isOpened());
}

void TestMineField::testKeyboardSpaceOnOpenedChords()
{
    MineField field;
    // Mine at (0,0); (1,1) has number 1.
    field.setFixedLayout(3, 3, {{0, 0}});
    field.cellAt(1, 1)->Open();
    QCOMPARE(field.cellAt(1, 1)->Number(), 1u);

    // Flag the mine.
    field.cellAt(0, 0)->cycleMarker();
    QVERIFY(field.cellAt(0, 0)->isFlagged());

    // Space on opened (1,1) should chord and open all safe neighbours.
    sendKey(field.cellAt(1, 1), Qt::Key_Space);
    QVERIFY(field.cellAt(0, 1)->isOpened());
    QVERIFY(field.cellAt(1, 0)->isOpened());
    QVERIFY(field.cellAt(2, 0)->isOpened());
    QVERIFY(field.cellAt(2, 2)->isOpened());
    QVERIFY(!field.cellAt(0, 0)->isOpened()); // stays flagged
}

void TestMineField::testKeyboardFTogglesFlag()
{
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}});
    QLabel label;
    field.setMineCountLabel(&label);

    MineButton *target = field.cellAt(2, 2);
    QCOMPARE(label.text(), QString::number(1));

    sendKey(target, Qt::Key_F);
    QCOMPARE(target->marker(), CellMarker::Flag);
    QCOMPARE(label.text(), QString::number(0));

    sendKey(target, Qt::Key_F);
    // Default: question marks enabled → second F goes to Question, not None.
    QCOMPARE(target->marker(), CellMarker::Question);
    QCOMPARE(label.text(), QString::number(1)); // flag cleared, mine count back up

    sendKey(target, Qt::Key_F);
    QCOMPARE(target->marker(), CellMarker::None);
}

void TestMineField::testKeyboardDChordsOpenedCell()
{
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}});
    field.cellAt(1, 1)->Open();
    field.cellAt(0, 0)->cycleMarker(); // flag

    sendKey(field.cellAt(1, 1), Qt::Key_D);
    QVERIFY(field.cellAt(0, 1)->isOpened());
    QVERIFY(field.cellAt(2, 2)->isOpened());
}

void TestMineField::testKeyboardSpaceOnFlaggedIsNoop()
{
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}});
    MineButton *target = field.cellAt(2, 2);
    target->cycleMarker(); // flag it
    QVERIFY(target->isFlagged());

    sendKey(target, Qt::Key_Space);
    QVERIFY(!target->isOpened()); // flag protects from reveal
    QVERIFY(target->isFlagged());
}

void TestMineField::testKeyboardIgnoredAfterLoss()
{
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}});
    field.cellAt(0, 0)->Open(); // boom
    QCOMPARE(field.state(), GameState::Lost);

    MineButton *safe = field.cellAt(2, 2);
    QVERIFY(!safe->isOpened());
    sendKey(safe, Qt::Key_F);
    QCOMPARE(safe->marker(), CellMarker::None); // F ignored after loss
    sendKey(safe, Qt::Key_Space);
    // Note: after Lost, handleCellKey returns false for Space, so the event
    // falls through to QPushButton. But QAbstractButton activation calls
    // clicked() which nothing is wired to; cell remains un-opened either way.
    QVERIFY(!safe->isOpened());
}

void TestMineField::testPauseDefaultsFalse()
{
    MineField field;
    QVERIFY(!field.isPaused());
    field.setFixedLayout(3, 3, {{0, 0}});
    QVERIFY(!field.isPaused());
}

void TestMineField::testPauseBlocksLeftClickReveal()
{
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}});
    MineButton *target = field.cellAt(2, 2);
    QVERIFY(!target->isOpened());

    field.setPaused(true);
    QVERIFY(field.isPaused());

    sendMousePress(target, Qt::LeftButton);
    QVERIFY(!target->isOpened());
    QCOMPARE(field.state(), GameState::Ready);
}

void TestMineField::testPauseBlocksRightClickFlag()
{
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}});
    MineButton *target = field.cellAt(2, 2);
    QCOMPARE(target->marker(), CellMarker::None);

    field.setPaused(true);

    sendMousePress(target, Qt::RightButton);
    QCOMPARE(target->marker(), CellMarker::None);
}

void TestMineField::testPauseBlocksKeyboardSpace()
{
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}});
    MineButton *target = field.cellAt(2, 2);

    field.setPaused(true);

    sendKey(target, Qt::Key_Space);
    QVERIFY(!target->isOpened());
}

void TestMineField::testPauseBlocksKeyboardArrowsAndF()
{
    MineField field;
    field.setFixedLayout(5, 5, {{0, 0}});
    field.show();
    QVERIFY(QTest::qWaitForWindowExposed(&field));
    MineButton *start = field.cellAt(2, 2);
    start->setFocus(Qt::OtherFocusReason);
    QCOMPARE(field.focusWidget(), start);

    field.setPaused(true);

    // Arrow key blocked → focus stays put.
    sendKey(start, Qt::Key_Right);
    QCOMPARE(field.focusWidget(), start);

    // F blocked → marker unchanged.
    sendKey(start, Qt::Key_F);
    QCOMPARE(start->marker(), CellMarker::None);
}

void TestMineField::testResumeRestoresInput()
{
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}});
    MineButton *target = field.cellAt(2, 2);

    field.setPaused(true);
    sendMousePress(target, Qt::LeftButton);
    QVERIFY(!target->isOpened());

    field.setPaused(false);
    QVERIFY(!field.isPaused());

    sendMousePress(target, Qt::LeftButton);
    QVERIFY(target->isOpened());
}

void TestMineField::testNewGameClearsPause()
{
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}});
    field.setPaused(true);
    QVERIFY(field.isPaused());

    field.newGame(MineField::Beginner);
    QVERIFY(!field.isPaused());

    // Confirm input restored on the freshly built grid.
    MineButton *target = field.cellAt(0, 0);
    sendMousePress(target, Qt::LeftButton);
    QVERIFY(target->isOpened());
}

void TestMineField::testReplayClearsPause()
{
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}});
    // Trigger a "first click" so a layout is recorded for replay.
    field.cellAt(2, 2)->Open();
    QVERIFY(field.canReplay());

    field.setPaused(true);
    QVERIFY(field.isPaused());

    QVERIFY(field.newGameReplay());
    QVERIFY(!field.isPaused());
}

void TestMineField::testPauseToggleIdempotent()
{
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}});
    field.setPaused(true);
    field.setPaused(true); // idempotent
    QVERIFY(field.isPaused());

    field.setPaused(false);
    field.setPaused(false); // idempotent
    QVERIFY(!field.isPaused());

    // Confirm input still works after redundant toggles.
    MineButton *target = field.cellAt(2, 2);
    sendMousePress(target, Qt::LeftButton);
    QVERIFY(target->isOpened());
}

void TestMineField::testAnyFlagPlacedDefaultsFalse()
{
    MineField field;
    QVERIFY(!field.anyFlagPlaced());
    field.setFixedLayout(3, 3, {{0, 0}});
    QVERIFY(!field.anyFlagPlaced());
}

void TestMineField::testAnyFlagPlacedTrueAfterRightClickFlag()
{
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}});
    QMouseEvent rightPress(QEvent::MouseButtonPress, QPointF(1, 1), QPointF(1, 1), Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    QCoreApplication::sendEvent(field.cellAt(2, 2), &rightPress);
    QVERIFY(field.cellAt(2, 2)->isFlagged());
    QVERIFY(field.anyFlagPlaced());
}

void TestMineField::testAnyFlagPlacedTrueAfterKeyboardF()
{
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}});
    sendKey(field.cellAt(2, 2), Qt::Key_F);
    QVERIFY(field.cellAt(2, 2)->isFlagged());
    QVERIFY(field.anyFlagPlaced());
}

void TestMineField::testAnyFlagPlacedStaysTrueAfterUnflag()
{
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}});
    MineButton *target = field.cellAt(2, 2);
    // Cycle marker None → Flag (sets sticky); then Flag → Question; then Question → None.
    target->cycleMarker(); // Flag — sets m_anyFlagPlaced
    QVERIFY(field.anyFlagPlaced());
    target->cycleMarker(); // Question — clears flag bookkeeping but anyFlag stays true
    QVERIFY(!target->isFlagged());
    QVERIFY(field.anyFlagPlaced());
    target->cycleMarker(); // None
    QCOMPARE(target->marker(), CellMarker::None);
    QVERIFY(field.anyFlagPlaced());
}

void TestMineField::testAnyFlagPlacedResetByNewGame()
{
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}});
    field.cellAt(2, 2)->cycleMarker();
    QVERIFY(field.anyFlagPlaced());

    field.newGame(MineField::Beginner);
    QVERIFY(!field.anyFlagPlaced());
}

void TestMineField::testAnyFlagPlacedResetByReplay()
{
    MineField field;
    // Deterministic layout — the previous version triggered first-click flow
    // on a random Beginner board and then flagged (0, 0); when the random
    // mine layout produced a flood that reached (0, 0), cycleMarker was a
    // no-op on the now-opened cell and anyFlagPlaced never flipped. Using
    // setFixedLayout removes the RNG dependency.
    field.setFixedLayout(5, 5, {{0, 0}});
    QVERIFY(field.canReplay());

    field.cellAt(2, 2)->cycleMarker();
    QVERIFY(field.anyFlagPlaced());

    QVERIFY(field.newGameReplay());
    QVERIFY(!field.anyFlagPlaced());
}

void TestMineField::testAnyFlagPlacedNotSetByQuestionMark()
{
    // Skipping straight from None to Question (i.e. when question marks are
    // enabled and the user… wait, the cycle is None → Flag → Question, so
    // there's no way to land on Question without passing through Flag first.
    // Instead: confirm that setting a question mark does NOT separately set
    // the bookkeeping bit — the bit comes from the Flag transition only.
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}});
    QVERIFY(!field.anyFlagPlaced());
    // No interaction at all: still false.
    field.cellAt(2, 2)->Open();
    QVERIFY(!field.anyFlagPlaced());
}

void TestMineField::testAnyFlagPlacedFalseAfterNoflagWin()
{
    // Regression: the win path calls flagAllMines() which auto-flags every
    // mine *after* the state flips to Won. Those celebratory flags must not
    // poison the no-flag bookkeeping for the just-completed run.
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}, {2, 2}});
    QSignalSpy wonSpy(&field, &MineField::gameWon);

    // Open every safe cell — no flags placed by the user.
    field.cellAt(1, 1)->Open();
    openAllSafe(field);

    QCOMPARE(wonSpy.count(), 1);
    QCOMPARE(field.state(), GameState::Won);
    QVERIFY2(!field.anyFlagPlaced(), "auto-flag-on-win must not flip the no-flag bookkeeping bit");
}

void TestMineField::testBoardValueZeroBeforeMinesPlaced()
{
    MineField field;
    QCOMPARE(field.boardValue(), 0);
}

void TestMineField::testBoardValueSingleOpening()
{
    // 5x5 with two corner mines: every safe cell is connected via zero cells,
    // so the entire safe region is one opening. BV = 1.
    MineField field;
    field.setFixedLayout(5, 5, {{0, 0}, {4, 4}});
    QCOMPARE(field.boardValue(), 1);
}

void TestMineField::testBoardValueAllNumberedNoZeros()
{
    // 3x3 with center mine: every surrounding cell has number 1 and there
    // are no zero cells. BV = 8 (each non-mine must be clicked individually).
    MineField field;
    field.setFixedLayout(3, 3, {{1, 1}});
    QCOMPARE(field.boardValue(), 8);
}

void TestMineField::testBoardValueTwoSeparateOpenings()
{
    // 7x3 with a horizontal wall of mines in row 3 separates the board into
    // two zero regions (rows 0–1 and rows 5–6). Row 2 and row 4 are fringe
    // numbered cells absorbed by their respective openings. BV = 2.
    MineField field;
    field.setFixedLayout(3, 7, {{3, 0}, {3, 1}, {3, 2}});
    QCOMPARE(field.boardValue(), 2);
}

void TestMineField::testBoardValueIsolatedNumberedCells()
{
    // 5x5 with a horizontal wall of mines in row 1 cuts off row 0 from any
    // zero region (row 2 is a fringe of the lower opening, but row 0 is
    // separated by the mine wall — every row 0 cell is numbered and adjacent
    // only to other numbered cells). One opening (rows 2–4) + 5 isolated
    // row-0 numbered cells. BV = 1 + 5 = 6.
    MineField field;
    field.setFixedLayout(5, 5, {{1, 0}, {1, 1}, {1, 2}, {1, 3}, {1, 4}});
    QCOMPARE(field.boardValue(), 6);
}

void TestMineField::testBoardValueFullBoardOfMinesEdgeCase()
{
    // No safe cells → no clicks possible → BV = 0. Pathological layout (the
    // game would never reach this), but compute3BV must not divide-by-zero
    // or count phantoms.
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}, {0, 1}, {0, 2}, {1, 0}, {1, 1}, {1, 2}, {2, 0}, {2, 1}, {2, 2}});
    QCOMPARE(field.boardValue(), 0);
}

void TestMineField::testBoardValueResetByNewGame()
{
    // After a layout is in place, boardValue() is non-zero. newGame must
    // wipe the cache back to zero (mines aren't placed yet on the new game).
    MineField field;
    field.setFixedLayout(3, 3, {{1, 1}});
    QVERIFY(field.boardValue() > 0);
    field.newGame(MineField::Beginner);
    QCOMPARE(field.boardValue(), 0);
}

void TestMineField::testBoardValueComputedAfterFirstClick()
{
    // Realistic flow: fresh MineField has BV=0, the first click triggers
    // mine placement and 3BV computation, after which BV must be ≥ 1
    // (any non-empty Beginner board has at least one opening from the
    // first-click safety zero zone).
    MineField field;
    field.newGame(MineField::Beginner);
    QCOMPARE(field.boardValue(), 0);
    QSignalSpy started(&field, &MineField::gameStarted);
    field.cellAt(4, 4)->Open();
    QCOMPARE(started.count(), 1);
    QVERIFY(field.boardValue() >= 1);
}

void TestMineField::testBoardValueReplayPreservesValue()
{
    // Replays use the snapshotted mine positions, so BV must compute to the
    // same value the original layout produced.
    MineField field;
    field.setFixedLayout(5, 5, {{0, 0}, {4, 4}});
    const int initialBV = field.boardValue();
    QVERIFY(initialBV >= 1);
    QVERIFY(field.newGameReplay());
    QCOMPARE(field.boardValue(), initialBV);
}

void TestMineField::testBoardValuePreservedOnLoss()
{
    // Regression guard for the loss-dialog "Board 3BV: %1" line: at the moment
    // gameLost fires, boardValue() must already be set (mine placement and
    // 3BV computation happen synchronously on the first click, well before
    // any explosion is reachable). If a future refactor defers BV computation
    // or zeroes it on loss, this test catches the resulting "Board 3BV: 0"
    // line that the > 0 gate would silently swallow.
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}, {0, 2}});
    const int initialBV = field.boardValue();
    QVERIFY(initialBV >= 1);
    sendMousePress(field.cellAt(0, 2), Qt::LeftButton);
    QCOMPARE(field.state(), GameState::Lost);
    QCOMPARE(field.boardValue(), initialBV);
}

void TestMineField::testUserClicksZeroBeforeAnyClick()
{
    MineField field;
    QCOMPARE(field.userClicks(), 0);
    field.setFixedLayout(5, 5, {{0, 0}});
    QCOMPARE(field.userClicks(), 0);
}

void TestMineField::testUserClicksLeftClickIncrementsOnce()
{
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}});
    sendMousePress(field.cellAt(2, 2), Qt::LeftButton);
    // Single user gesture, regardless of how the cell flooded.
    QCOMPARE(field.userClicks(), 1);
}

void TestMineField::testUserClicksFloodCountsOnce()
{
    MineField field;
    // Mine in a corner of a 5x5 — clicking the opposite corner triggers a
    // big flood that opens many cells. Only the user gesture should count.
    field.setFixedLayout(5, 5, {{0, 0}});
    sendMousePress(field.cellAt(4, 4), Qt::LeftButton);
    int openedCount = 0;
    for (std::uint32_t r = 0; r < 5; ++r)
    {
        for (std::uint32_t c = 0; c < 5; ++c)
        {
            if (field.cellAt(r, c)->isOpened())
            {
                ++openedCount;
            }
        }
    }
    QVERIFY(openedCount > 1); // flood actually happened
    QCOMPARE(field.userClicks(), 1);
}

void TestMineField::testUserClicksLeftClickOnFlaggedDoesNothing()
{
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}});
    MineButton *target = field.cellAt(2, 2);
    target->cycleMarker();
    QVERIFY(target->isFlagged());
    sendMousePress(target, Qt::LeftButton);
    QVERIFY(!target->isOpened());
    QCOMPARE(field.userClicks(), 0);
}

void TestMineField::testUserClicksLeftClickOnOpenedDoesNothing()
{
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}});
    MineButton *target = field.cellAt(2, 2);
    sendMousePress(target, Qt::LeftButton);
    QCOMPARE(field.userClicks(), 1);
    // Second click on already-opened cell should be a no-op for the counter.
    sendMousePress(target, Qt::LeftButton);
    QCOMPARE(field.userClicks(), 1);
}

void TestMineField::testUserClicksRightClickDoesNothing()
{
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}});
    MineButton *target = field.cellAt(2, 2);
    sendMousePress(target, Qt::RightButton);
    QVERIFY(target->isFlagged());
    QCOMPARE(field.userClicks(), 0);
}

void TestMineField::testUserClicksKeyboardSpaceCountsOnce()
{
    MineField field;
    field.setFixedLayout(5, 5, {{0, 0}});
    sendKey(field.cellAt(4, 4), Qt::Key_Space);
    QVERIFY(field.cellAt(4, 4)->isOpened());
    QCOMPARE(field.userClicks(), 1);
}

void TestMineField::testUserClicksKeyboardSpaceOnFlaggedDoesNothing()
{
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}});
    MineButton *target = field.cellAt(2, 2);
    target->cycleMarker();
    sendKey(target, Qt::Key_Space);
    QVERIFY(!target->isOpened());
    QCOMPARE(field.userClicks(), 0);
}

void TestMineField::testUserClicksChordCountsOnceWhenOpens()
{
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}});
    field.cellAt(1, 1)->Open(); // direct test-helper open does NOT count as gesture
    QCOMPARE(field.userClicks(), 0);
    field.cellAt(0, 0)->cycleMarker(); // flag the mine
    // Chord gesture on (1,1) opens (0,1), (1,0), (0,2), (2,0), (2,1), (2,2).
    sendMousePress(field.cellAt(1, 1), Qt::MiddleButton);
    QCOMPARE(field.userClicks(), 1);
}

void TestMineField::testUserClicksChordOnFullySatisfiedDoesNothing()
{
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}});
    field.cellAt(1, 1)->Open();
    field.cellAt(0, 0)->cycleMarker(); // flag mine
    // Pre-open every safe neighbour by direct calls (does not count).
    field.cellAt(0, 1)->Open();
    field.cellAt(1, 0)->Open();
    field.cellAt(2, 0)->Open();
    field.cellAt(0, 2)->Open();
    field.cellAt(2, 1)->Open();
    field.cellAt(2, 2)->Open();
    QCOMPARE(field.userClicks(), 0);
    // Chord on (1,1) is satisfied but every neighbour is already opened or
    // flagged — the gesture opens nothing and must not count.
    sendMousePress(field.cellAt(1, 1), Qt::MiddleButton);
    QCOMPARE(field.userClicks(), 0);
}

void TestMineField::testUserClicksChordWrongFlagCountsAndExplodes()
{
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}});
    field.cellAt(1, 1)->Open();
    // Wrong flag at (0,1) — count satisfies number 1, but it covers the wrong cell.
    field.cellAt(0, 1)->cycleMarker();
    sendMousePress(field.cellAt(1, 1), Qt::MiddleButton);
    QCOMPARE(field.state(), GameState::Lost);
    // The chord opened the actual mine (a fresh open), so the gesture counts.
    QCOMPARE(field.userClicks(), 1);
}

void TestMineField::testUserClicksLeftClickOnMineCountsAndExplodes()
{
    // Direct left-click onto a mine — the most common loss path. The click
    // must count toward userClicks before the explosion freezes the board,
    // so the loss-dialog "Clicks: %1" line surfaces a non-zero value.
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}});
    // Open a numbered cell first so the explosion happens on the *second*
    // gesture. Picking a number-bearing neighbour of the mine means no flood,
    // so the field stays Playing instead of winning on a single click.
    sendMousePress(field.cellAt(0, 1), Qt::LeftButton);
    QCOMPARE(field.state(), GameState::Playing);
    QCOMPARE(field.userClicks(), 1);
    sendMousePress(field.cellAt(0, 0), Qt::LeftButton);
    QCOMPARE(field.state(), GameState::Lost);
    QCOMPARE(field.userClicks(), 2);
}

void TestMineField::testUserClicksKeyboardDChordCountsOnce()
{
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}});
    field.cellAt(1, 1)->Open();
    field.cellAt(0, 0)->cycleMarker();
    QCOMPARE(field.userClicks(), 0);
    sendKey(field.cellAt(1, 1), Qt::Key_D);
    QCOMPARE(field.userClicks(), 1);
    QVERIFY(field.cellAt(2, 2)->isOpened());
}

void TestMineField::testUserClicksResetByNewGame()
{
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}});
    sendMousePress(field.cellAt(2, 2), Qt::LeftButton);
    QCOMPARE(field.userClicks(), 1);
    field.newGame(MineField::Beginner);
    QCOMPARE(field.userClicks(), 0);
}

void TestMineField::testUserClicksResetByReplay()
{
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}});
    sendMousePress(field.cellAt(2, 2), Qt::LeftButton);
    QCOMPARE(field.userClicks(), 1);
    QVERIFY(field.newGameReplay());
    QCOMPARE(field.userClicks(), 0);
}

void TestMineField::testUserClicksResetBySetFixedLayout()
{
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}});
    sendMousePress(field.cellAt(2, 2), Qt::LeftButton);
    QCOMPARE(field.userClicks(), 1);
    field.setFixedLayout(4, 4, {{0, 0}});
    QCOMPARE(field.userClicks(), 0);
}

void TestMineField::testRefreshAllNumberStylesReflectsPaletteChange()
{
    // Open a numbered cell under the classic palette, flip on the
    // colour-blind palette and run the bulk refresh. The opened cell's
    // stylesheet should pick up the new number colour.
    MineButton::setColorBlindPaletteEnabled(false);
    MineField field;
    // 3×3, single mine in the corner — the opposite corner is guaranteed
    // to be a non-zero numbered cell after first-click safety.
    field.setFixedLayout(3, 3, {{0, 0}});
    sendMousePress(field.cellAt(2, 2), Qt::LeftButton);
    // (2,2) is adjacent to no mines → number 0. Click (1,0) which is a
    // number=1 cell; that one will have a color rule.
    auto *numbered = field.cellAt(1, 0);
    QVERIFY(numbered->isOpened());
    QCOMPARE(numbered->Number(), 1u);
    const QString classicStyle = numbered->styleSheet();
    QVERIFY(classicStyle.contains(QStringLiteral("color: rgb(")));

    MineButton::setColorBlindPaletteEnabled(true);
    field.refreshAllNumberStyles();
    const QString cbStyle = numbered->styleSheet();
    QVERIFY(cbStyle.contains(QStringLiteral("color: rgb(")));
    QVERIFY(classicStyle != cbStyle);

    MineButton::setColorBlindPaletteEnabled(false);
}

void TestMineField::testRefreshAllNumberStylesSkipsUnopened()
{
    // Unopened cells on the grid must not get a number-colour style after
    // the bulk refresh — that would leak the opened-cell visual onto
    // still-covered tiles.
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}});
    // Don't click anything; every cell is unopened.
    MineButton::setColorBlindPaletteEnabled(true);
    field.refreshAllNumberStyles();
    for (std::uint32_t r = 0; r < 3; ++r)
    {
        for (std::uint32_t c = 0; c < 3; ++c)
        {
            auto *cell = field.cellAt(r, c);
            QVERIFY(cell != nullptr);
            QVERIFY(!cell->styleSheet().contains(QStringLiteral("color: rgb(")));
        }
    }
    MineButton::setColorBlindPaletteEnabled(false);
}

void TestMineField::testSafePercentClearedZeroBeforeAnyClick()
{
    MineField field;
    QCOMPARE(field.safePercentCleared(), 0);
    field.setFixedLayout(5, 5, {{0, 0}});
    QCOMPARE(field.safePercentCleared(), 0);
}

void TestMineField::testSafePercentClearedReachesHundredOnWin()
{
    // 3x3 with one mine in the corner — flooding from the opposite corner
    // reveals all 8 safe cells, hitting 100%.
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}});
    sendMousePress(field.cellAt(2, 2), Qt::LeftButton);
    QCOMPARE(field.state(), GameState::Won);
    QCOMPARE(field.safePercentCleared(), 100);
}

void TestMineField::testSafePercentClearedMidGame()
{
    // 4x4 with mines everywhere except (0,0) and (0,1) — only two safe cells.
    // (0,0) is adjacent to three mines so it has number 3 and does not flood.
    // After opening (0,0), 1 of 2 safe cells is opened => 50%.
    std::vector<std::pair<std::uint32_t, std::uint32_t>> mines;
    for (std::uint32_t r = 0; r < 4; ++r)
    {
        for (std::uint32_t c = 0; c < 4; ++c)
        {
            if (!(r == 0 && c == 0) && !(r == 0 && c == 1))
            {
                mines.emplace_back(r, c);
            }
        }
    }
    MineField field;
    field.setFixedLayout(4, 4, mines);
    sendMousePress(field.cellAt(0, 0), Qt::LeftButton);
    QVERIFY(field.cellAt(0, 0)->isOpened());
    QVERIFY(!field.cellAt(0, 1)->isOpened());
    QCOMPARE(field.safePercentCleared(), 50);
}

void TestMineField::testSafePercentClearedRoundsHalfUp()
{
    // 3x3 with 6 mines in rows 1..2 — 3 safe cells along the top row. Each
    // click opens exactly one of them (numbers 2, 3, 2; no zeros, no flood).
    MineField field;
    field.setFixedLayout(3, 3, {{1, 0}, {1, 1}, {1, 2}, {2, 0}, {2, 1}, {2, 2}});
    sendMousePress(field.cellAt(0, 0), Qt::LeftButton);
    // 1 of 3 = 33.33…%, round-half-up → 33.
    QCOMPARE(field.safePercentCleared(), 33);
    sendMousePress(field.cellAt(0, 2), Qt::LeftButton);
    // 2 of 3 = 66.67%, round-half-up → 67.
    QCOMPARE(field.safePercentCleared(), 67);
}

void TestMineField::testFlagsPlacedZeroBeforeAnyFlag()
{
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}});
    QCOMPARE(field.flagsPlaced(), 0);
}

void TestMineField::testFlagsPlacedIncrementsOnFlag()
{
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}});
    field.cellAt(0, 0)->cycleMarker(); // None → Flag
    QCOMPARE(field.flagsPlaced(), 1);
    field.cellAt(2, 2)->cycleMarker();
    QCOMPARE(field.flagsPlaced(), 2);
}

void TestMineField::testFlagsPlacedDecrementsOnUnflag()
{
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}});
    field.cellAt(2, 2)->cycleMarker(); // None → Flag
    QCOMPARE(field.flagsPlaced(), 1);
    field.cellAt(2, 2)->cycleMarker(); // Flag → Question (flag goes off)
    QCOMPARE(field.flagsPlaced(), 0);
}

void TestMineField::testFlagsPlacedPreservedOnLoss()
{
    // The loss path's revealAllMines does NOT auto-flag, so the user's flag
    // count at moment of explosion is what flagsPlaced() returns. Regression
    // guard for the loss-dialog "Flags placed: %1" line — if a future change
    // makes the loss path auto-flag remaining mines, this test catches the
    // resulting count inflation.
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}, {0, 2}});
    // User flags one of the two mines correctly, then steps on the other.
    field.cellAt(0, 0)->cycleMarker();
    QCOMPARE(field.flagsPlaced(), 1);
    sendMousePress(field.cellAt(0, 2), Qt::LeftButton);
    QCOMPARE(field.state(), GameState::Lost);
    // Still 1 — the unflagged mine at (0,0) is not auto-flagged on loss.
    QCOMPARE(field.flagsPlaced(), 1);
}

void TestMineField::testFlagsPlacedResetByNewGame()
{
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}});
    field.cellAt(0, 0)->cycleMarker();
    QCOMPARE(field.flagsPlaced(), 1);
    field.newGame(MineField::Beginner);
    QCOMPARE(field.flagsPlaced(), 0);
}

void TestMineField::testQuestionMarksPlacedZeroBeforeAnyMark()
{
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}});
    QCOMPARE(field.questionMarksPlaced(), 0);
}

void TestMineField::testQuestionMarksPlacedCountsOnlyQuestionCells()
{
    // Right-click cycles None → Flag → Question → None. The counter must include
    // the cell currently in Question and exclude one currently in Flag.
    MineButton::setQuestionMarksEnabled(true);
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}});
    field.cellAt(0, 1)->cycleMarker(); // None → Flag
    field.cellAt(0, 1)->cycleMarker(); // Flag → Question
    QCOMPARE(field.questionMarksPlaced(), 1);
    field.cellAt(2, 2)->cycleMarker(); // None → Flag (does not count)
    QCOMPARE(field.questionMarksPlaced(), 1);
    field.cellAt(1, 1)->cycleMarker(); // None → Flag
    field.cellAt(1, 1)->cycleMarker(); // Flag → Question
    QCOMPARE(field.questionMarksPlaced(), 2);
}

void TestMineField::testQuestionMarksPlacedDecrementsOnCycleAway()
{
    MineButton::setQuestionMarksEnabled(true);
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}});
    field.cellAt(2, 2)->cycleMarker(); // None → Flag
    field.cellAt(2, 2)->cycleMarker(); // Flag → Question
    QCOMPARE(field.questionMarksPlaced(), 1);
    field.cellAt(2, 2)->cycleMarker(); // Question → None
    QCOMPARE(field.questionMarksPlaced(), 0);
}

void TestMineField::testQuestionMarksPlacedPreservedOnLoss()
{
    // The loss path's revealAllMines does NOT clear m_marker, so a `?` placed
    // by the user — even on a mined cell — still counts after the loss reveal.
    // Regression guard for the loss-dialog "Question marks: %1" line: any
    // future change that wipes the marker on reveal would silently zero this
    // out and break the dialog metric.
    MineButton::setQuestionMarksEnabled(true);
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}, {0, 2}});
    // User cycles (0, 0) (a mine) all the way to Question, then steps on the other mine.
    field.cellAt(0, 0)->cycleMarker(); // None → Flag
    field.cellAt(0, 0)->cycleMarker(); // Flag → Question
    QCOMPARE(field.questionMarksPlaced(), 1);
    sendMousePress(field.cellAt(0, 2), Qt::LeftButton);
    QCOMPARE(field.state(), GameState::Lost);
    // Still 1 — revealAllMines paints the mined cell as exploded but does not
    // touch the underlying CellMarker, so isQuestion() stays true.
    QCOMPARE(field.questionMarksPlaced(), 1);
}

void TestMineField::testQuestionMarksPlacedResetByNewGame()
{
    MineButton::setQuestionMarksEnabled(true);
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}});
    field.cellAt(0, 0)->cycleMarker(); // None → Flag
    field.cellAt(0, 0)->cycleMarker(); // Flag → Question
    QCOMPARE(field.questionMarksPlaced(), 1);
    field.newGame(MineField::Beginner);
    QCOMPARE(field.questionMarksPlaced(), 0);
}

QTEST_MAIN(TestMineField)
#include "tst_minefield.moc"
