#include "../minebutton.h"
#include "../minefield.h"
#include "../smiley.h"

#include <QCoreApplication>
#include <QMouseEvent>
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
    void testTensionFaceWhilePressing();
    void testTensionIgnoredAfterGameOver();
    void testPressReleaseDrivesTension();
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

void TestSmiley::testTensionFaceWhilePressing()
{
    // While a cell is held down in Ready or Playing, the indicator flips to
    // the classic 😮 "holding my breath" face.
    QCOMPARE(smileyForTensionState(GameState::Ready, true), QStringLiteral("😮"));
    QCOMPARE(smileyForTensionState(GameState::Playing, true), QStringLiteral("😮"));
    // Not pressing — falls back to the plain state face.
    QCOMPARE(smileyForTensionState(GameState::Ready, false), QStringLiteral("🙂"));
    QCOMPARE(smileyForTensionState(GameState::Playing, false), QStringLiteral("🙂"));
}

void TestSmiley::testTensionIgnoredAfterGameOver()
{
    // After win/loss the cells are frozen, so any lingering "pressing" flag
    // must not override the final 😎/😵 — the indicator should stay put.
    QCOMPARE(smileyForTensionState(GameState::Won, true), QStringLiteral("😎"));
    QCOMPARE(smileyForTensionState(GameState::Lost, true), QStringLiteral("😵"));
    QCOMPARE(smileyForTensionState(GameState::Won, false), QStringLiteral("😎"));
    QCOMPARE(smileyForTensionState(GameState::Lost, false), QStringLiteral("😵"));
}

void TestSmiley::testPressReleaseDrivesTension()
{
    // Full wiring: MineButton press/release → MineField forwards →
    // button text flips between 🙂 and 😮 for left-click, and stays on 🙂 for
    // right-click (flag-only presses must not trigger tension).
    MineField field;
    field.setFixedLayout(3, 3, {{0, 0}});
    GameState lastState = GameState::Ready;
    bool pressing = false;
    QPushButton button;
    const auto refresh = [&] { button.setText(smileyForTensionState(lastState, pressing)); };
    refresh();

    QObject::connect(&field, &MineField::gameStarted, &button,
                     [&]
                     {
                         lastState = GameState::Playing;
                         refresh();
                     });
    QObject::connect(&field, &MineField::cellInteractionStarted, &button,
                     [&]
                     {
                         pressing = true;
                         refresh();
                     });
    QObject::connect(&field, &MineField::cellInteractionEnded, &button,
                     [&]
                     {
                         pressing = false;
                         refresh();
                     });

    auto *cell = field.cellAt(1, 1);
    QVERIFY(cell != nullptr);

    // Left-click: press → tension. Release → tension off, state=Playing.
    QCOMPARE(button.text(), QStringLiteral("🙂"));
    QMouseEvent press(QEvent::MouseButtonPress, QPointF(5, 5), QPointF(5, 5), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(cell, &press);
    QCOMPARE(button.text(), QStringLiteral("😮"));
    QMouseEvent release(QEvent::MouseButtonRelease, QPointF(5, 5), QPointF(5, 5), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    QCoreApplication::sendEvent(cell, &release);
    QCOMPARE(button.text(), QStringLiteral("🙂"));
    QCOMPARE(field.state(), GameState::Playing);

    // Right-click on an un-opened neighbour: no tension at any point.
    auto *neighbour = field.cellAt(2, 2);
    QVERIFY(neighbour != nullptr);
    QMouseEvent rpress(QEvent::MouseButtonPress, QPointF(5, 5), QPointF(5, 5), Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    QCoreApplication::sendEvent(neighbour, &rpress);
    QCOMPARE(button.text(), QStringLiteral("🙂"));
    QMouseEvent rrelease(QEvent::MouseButtonRelease, QPointF(5, 5), QPointF(5, 5), Qt::RightButton, Qt::NoButton, Qt::NoModifier);
    QCoreApplication::sendEvent(neighbour, &rrelease);
    QCOMPARE(button.text(), QStringLiteral("🙂"));
}

QTEST_MAIN(TestSmiley)
#include "tst_smiley.moc"
