#include "../minebutton.h"
#include "../minefield.h"
#include "../smiley.h"

#include <QPushButton>
#include <QtTest>

class TestSmiley : public QObject
{
    Q_OBJECT

  private slots:
    void testReadyFace();
    void testPlayingFace();
    void testWonFace();
    void testLostFace();
    void testAllFacesAreDistinctExceptReadyPlaying();
    void testIntegrationWithMineFieldSignals();
};

void TestSmiley::testReadyFace() { QCOMPARE(smileyForState(GameState::Ready), QStringLiteral("🙂")); }

void TestSmiley::testPlayingFace() { QCOMPARE(smileyForState(GameState::Playing), QStringLiteral("🙂")); }

void TestSmiley::testWonFace() { QCOMPARE(smileyForState(GameState::Won), QStringLiteral("😎")); }

void TestSmiley::testLostFace() { QCOMPARE(smileyForState(GameState::Lost), QStringLiteral("😵")); }

void TestSmiley::testAllFacesAreDistinctExceptReadyPlaying()
{
    // Ready and Playing intentionally share the neutral face — only the
    // end-of-game states (Won/Lost) are visually distinct. Guard against a
    // future refactor that accidentally collapses Won and Lost into the same
    // emoji.
    const QString ready = smileyForState(GameState::Ready);
    const QString playing = smileyForState(GameState::Playing);
    const QString won = smileyForState(GameState::Won);
    const QString lost = smileyForState(GameState::Lost);

    QCOMPARE(ready, playing);
    QVERIFY(ready != won);
    QVERIFY(ready != lost);
    QVERIFY(won != lost);
}

void TestSmiley::testIntegrationWithMineFieldSignals()
{
    // Mirror the wiring MainWindow does: a QPushButton whose text tracks the
    // three MineField signals. Drives the real MineField through start → loss
    // to prove the button text updates end-to-end — regression catch if a
    // signal gets renamed or a connection gets dropped.
    MineField field;
    QPushButton button;
    button.setText(smileyForState(GameState::Ready));
    QCOMPARE(button.text(), QStringLiteral("🙂"));

    QObject::connect(&field, &MineField::gameStarted, &button, [&button] { button.setText(smileyForState(GameState::Playing)); });
    QObject::connect(&field, &MineField::gameWon, &button, [&button] { button.setText(smileyForState(GameState::Won)); });
    QObject::connect(&field, &MineField::gameLost, &button, [&button](std::uint32_t, std::uint32_t) { button.setText(smileyForState(GameState::Lost)); });

    // Deterministic 3×3 layout with a single mine at (0,0). Click (1,1) —
    // adjacent to the mine, number=1, so no flood-fill win. State goes to
    // Playing. Then step on (0,0) to fire gameLost.
    field.setFixedLayout(3, 3, {{0, 0}});

    field.cellAt(1, 1)->Open();
    QCOMPARE(field.state(), GameState::Playing);
    QCOMPARE(button.text(), QStringLiteral("🙂"));

    field.cellAt(0, 0)->Open();
    QCOMPARE(field.state(), GameState::Lost);
    QCOMPARE(button.text(), QStringLiteral("😵"));
}

QTEST_MAIN(TestSmiley)
#include "tst_smiley.moc"
