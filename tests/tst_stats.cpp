#include "../stats.h"

#include <QCoreApplication>
#include <QSettings>
#include <QStandardPaths>
#include <QtTest>

class TestStats : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void cleanup();

    void testEmptyRecordDefaults();
    void testRecordLossIncrementsPlayedOnly();
    void testRecordWinIncrementsPlayedAndWon();
    void testRecordWinSetsBestOnFirstWin();
    void testFasterWinBeatsBest();
    void testSlowerWinKeepsBest();
    void testPerDifficultyIsolation();
    void testResetSingle();
    void testResetAllWipesEverything();
};

void TestStats::initTestCase()
{
    QCoreApplication::setOrganizationName("Mavrikant");
    QCoreApplication::setApplicationName("QMineSweeperTestStats");
    QSettings::setDefaultFormat(QSettings::IniFormat);
}

void TestStats::cleanup()
{
    // Wipe settings between tests so each one starts fresh.
    QSettings settings;
    settings.clear();
}

void TestStats::testEmptyRecordDefaults()
{
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.played, 0u);
    QCOMPARE(r.won, 0u);
    QCOMPARE(r.bestSeconds, 0.0);
}

void TestStats::testRecordLossIncrementsPlayedOnly()
{
    Stats::recordLoss(QStringLiteral("Beginner"));
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.played, 1u);
    QCOMPARE(r.won, 0u);
    QCOMPARE(r.bestSeconds, 0.0);
}

void TestStats::testRecordWinIncrementsPlayedAndWon()
{
    const bool newRecord = Stats::recordWin(QStringLiteral("Beginner"), 15.5);
    QVERIFY(newRecord);
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.played, 1u);
    QCOMPARE(r.won, 1u);
    QCOMPARE(r.bestSeconds, 15.5);
}

void TestStats::testRecordWinSetsBestOnFirstWin()
{
    QVERIFY(Stats::recordWin(QStringLiteral("Beginner"), 30.0));
    QCOMPARE(Stats::load(QStringLiteral("Beginner")).bestSeconds, 30.0);
}

void TestStats::testFasterWinBeatsBest()
{
    QVERIFY(Stats::recordWin(QStringLiteral("Beginner"), 30.0));
    const bool nr = Stats::recordWin(QStringLiteral("Beginner"), 20.0);
    QVERIFY(nr);
    QCOMPARE(Stats::load(QStringLiteral("Beginner")).bestSeconds, 20.0);
}

void TestStats::testSlowerWinKeepsBest()
{
    QVERIFY(Stats::recordWin(QStringLiteral("Beginner"), 20.0));
    const bool nr = Stats::recordWin(QStringLiteral("Beginner"), 40.0);
    QVERIFY(!nr);
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.bestSeconds, 20.0);
    QCOMPARE(r.won, 2u);
    QCOMPARE(r.played, 2u);
}

void TestStats::testPerDifficultyIsolation()
{
    Stats::recordWin(QStringLiteral("Beginner"), 10.0);
    Stats::recordWin(QStringLiteral("Expert"), 300.0);

    QCOMPARE(Stats::load(QStringLiteral("Beginner")).bestSeconds, 10.0);
    QCOMPARE(Stats::load(QStringLiteral("Intermediate")).played, 0u);
    QCOMPARE(Stats::load(QStringLiteral("Expert")).bestSeconds, 300.0);
}

void TestStats::testResetSingle()
{
    Stats::recordWin(QStringLiteral("Beginner"), 10.0);
    Stats::recordWin(QStringLiteral("Expert"), 300.0);
    Stats::reset(QStringLiteral("Beginner"));
    QCOMPARE(Stats::load(QStringLiteral("Beginner")).played, 0u);
    QCOMPARE(Stats::load(QStringLiteral("Expert")).played, 1u);
}

void TestStats::testResetAllWipesEverything()
{
    Stats::recordWin(QStringLiteral("Beginner"), 10.0);
    Stats::recordWin(QStringLiteral("Intermediate"), 100.0);
    Stats::recordWin(QStringLiteral("Expert"), 300.0);
    Stats::resetAll();
    QCOMPARE(Stats::load(QStringLiteral("Beginner")).played, 0u);
    QCOMPARE(Stats::load(QStringLiteral("Intermediate")).played, 0u);
    QCOMPARE(Stats::load(QStringLiteral("Expert")).played, 0u);
}

QTEST_MAIN(TestStats)
#include "tst_stats.moc"
