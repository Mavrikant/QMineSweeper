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

  private:
    static void openAllSafe(MineField &field);
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

QTEST_MAIN(TestMineField)
#include "tst_minefield.moc"
