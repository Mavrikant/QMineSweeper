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

    // best-date behaviour
    void testBestDateInvalidWhenNoWins();
    void testFirstWinStampsDate();
    void testFasterWinOverwritesDate();
    void testSlowerWinKeepsOriginalDate();
    void testLossDoesNotTouchDate();
    void testResetWipesDate();
    void testLegacyRecordWithoutDateLoadsAsInvalid();

    // no-flag best-time bracket
    void testNoflagDefaultsZero();
    void testRecordNoflagBestSetsOnFirstCall();
    void testFasterNoflagBeatsAndStampsDate();
    void testSlowerNoflagKeepsOriginal();
    void testRecordNoflagBestDoesNotTouchOverall();
    void testRecordWinDoesNotTouchNoflag();
    void testResetWipesNoflag();
    void testResetAllWipesNoflag();
    void testLegacyRecordWithoutNoflagLoadsAsZero();
    void testRecordNoflagBestZeroSecondsRejected();
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

void TestStats::testBestDateInvalidWhenNoWins()
{
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QVERIFY(!r.bestDate.isValid());
}

void TestStats::testFirstWinStampsDate()
{
    const QDate d{2026, 4, 23};
    QVERIFY(Stats::recordWin(QStringLiteral("Beginner"), 30.0, d));
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.bestDate, d);
}

void TestStats::testFasterWinOverwritesDate()
{
    const QDate oldDate{2026, 1, 1};
    const QDate newDate{2026, 4, 23};
    QVERIFY(Stats::recordWin(QStringLiteral("Beginner"), 30.0, oldDate));
    QVERIFY(Stats::recordWin(QStringLiteral("Beginner"), 20.0, newDate));
    QCOMPARE(Stats::load(QStringLiteral("Beginner")).bestDate, newDate);
}

void TestStats::testSlowerWinKeepsOriginalDate()
{
    const QDate originalDate{2026, 1, 1};
    const QDate laterDate{2026, 4, 23};
    QVERIFY(Stats::recordWin(QStringLiteral("Beginner"), 20.0, originalDate));
    QVERIFY(!Stats::recordWin(QStringLiteral("Beginner"), 40.0, laterDate));
    QCOMPARE(Stats::load(QStringLiteral("Beginner")).bestDate, originalDate);
}

void TestStats::testLossDoesNotTouchDate()
{
    const QDate originalDate{2026, 1, 1};
    QVERIFY(Stats::recordWin(QStringLiteral("Beginner"), 20.0, originalDate));
    Stats::recordLoss(QStringLiteral("Beginner"));
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.bestDate, originalDate);
    QCOMPARE(r.played, 2u);
    QCOMPARE(r.won, 1u);
}

void TestStats::testResetWipesDate()
{
    QVERIFY(Stats::recordWin(QStringLiteral("Beginner"), 20.0, QDate{2026, 1, 1}));
    Stats::reset(QStringLiteral("Beginner"));
    QVERIFY(!Stats::load(QStringLiteral("Beginner")).bestDate.isValid());
}

void TestStats::testLegacyRecordWithoutDateLoadsAsInvalid()
{
    // Simulate a pre-1.3 record: played/won/best_seconds keys set, best_date absent.
    QSettings settings;
    settings.setValue(QStringLiteral("stats/Beginner/played"), 5u);
    settings.setValue(QStringLiteral("stats/Beginner/won"), 3u);
    settings.setValue(QStringLiteral("stats/Beginner/best_seconds"), 42.0);
    settings.sync();

    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.played, 5u);
    QCOMPARE(r.won, 3u);
    QCOMPARE(r.bestSeconds, 42.0);
    QVERIFY(!r.bestDate.isValid());
}

void TestStats::testNoflagDefaultsZero()
{
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.bestNoflagSeconds, 0.0);
    QVERIFY(!r.bestNoflagDate.isValid());
}

void TestStats::testRecordNoflagBestSetsOnFirstCall()
{
    const QDate d{2026, 4, 25};
    QVERIFY(Stats::recordNoflagBest(QStringLiteral("Beginner"), 12.3, d));
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.bestNoflagSeconds, 12.3);
    QCOMPARE(r.bestNoflagDate, d);
    // Played/won unchanged — recordNoflagBest is best-time only.
    QCOMPARE(r.played, 0u);
    QCOMPARE(r.won, 0u);
}

void TestStats::testFasterNoflagBeatsAndStampsDate()
{
    const QDate oldDate{2026, 1, 1};
    const QDate newDate{2026, 4, 25};
    QVERIFY(Stats::recordNoflagBest(QStringLiteral("Beginner"), 30.0, oldDate));
    QVERIFY(Stats::recordNoflagBest(QStringLiteral("Beginner"), 18.5, newDate));
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.bestNoflagSeconds, 18.5);
    QCOMPARE(r.bestNoflagDate, newDate);
}

void TestStats::testSlowerNoflagKeepsOriginal()
{
    const QDate originalDate{2026, 1, 1};
    QVERIFY(Stats::recordNoflagBest(QStringLiteral("Beginner"), 18.5, originalDate));
    QVERIFY(!Stats::recordNoflagBest(QStringLiteral("Beginner"), 40.0, QDate{2026, 4, 25}));
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.bestNoflagSeconds, 18.5);
    QCOMPARE(r.bestNoflagDate, originalDate);
}

void TestStats::testRecordNoflagBestDoesNotTouchOverall()
{
    // First seed an overall record at 30s.
    QVERIFY(Stats::recordWin(QStringLiteral("Beginner"), 30.0, QDate{2026, 1, 1}));
    // Now record a faster no-flag run at 20s. Overall best stays 30 because
    // recordNoflagBest is best-time only — it does not update bestSeconds.
    QVERIFY(Stats::recordNoflagBest(QStringLiteral("Beginner"), 20.0, QDate{2026, 4, 25}));
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.bestSeconds, 30.0);
    QCOMPARE(r.bestDate, (QDate{2026, 1, 1}));
    QCOMPARE(r.bestNoflagSeconds, 20.0);
    QCOMPARE(r.bestNoflagDate, (QDate{2026, 4, 25}));
    QCOMPARE(r.played, 1u);
    QCOMPARE(r.won, 1u);
}

void TestStats::testRecordWinDoesNotTouchNoflag()
{
    // Seed a no-flag best, then record a faster win-with-flags. The no-flag
    // bracket must not be overwritten by the regular win path.
    QVERIFY(Stats::recordNoflagBest(QStringLiteral("Beginner"), 25.0, QDate{2026, 1, 1}));
    QVERIFY(Stats::recordWin(QStringLiteral("Beginner"), 10.0, QDate{2026, 4, 25}));
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.bestSeconds, 10.0);
    QCOMPARE(r.bestNoflagSeconds, 25.0);
    QCOMPARE(r.bestNoflagDate, (QDate{2026, 1, 1}));
}

void TestStats::testResetWipesNoflag()
{
    QVERIFY(Stats::recordNoflagBest(QStringLiteral("Beginner"), 18.5, QDate{2026, 4, 25}));
    Stats::reset(QStringLiteral("Beginner"));
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.bestNoflagSeconds, 0.0);
    QVERIFY(!r.bestNoflagDate.isValid());
}

void TestStats::testResetAllWipesNoflag()
{
    QVERIFY(Stats::recordNoflagBest(QStringLiteral("Beginner"), 18.0, QDate{2026, 4, 25}));
    QVERIFY(Stats::recordNoflagBest(QStringLiteral("Expert"), 250.0, QDate{2026, 4, 25}));
    Stats::resetAll();
    QCOMPARE(Stats::load(QStringLiteral("Beginner")).bestNoflagSeconds, 0.0);
    QCOMPARE(Stats::load(QStringLiteral("Expert")).bestNoflagSeconds, 0.0);
}

void TestStats::testLegacyRecordWithoutNoflagLoadsAsZero()
{
    // Pre-1.13 record: no best_noflag_* keys present.
    QSettings settings;
    settings.setValue(QStringLiteral("stats/Beginner/played"), 5u);
    settings.setValue(QStringLiteral("stats/Beginner/won"), 3u);
    settings.setValue(QStringLiteral("stats/Beginner/best_seconds"), 42.0);
    settings.setValue(QStringLiteral("stats/Beginner/best_date"), QStringLiteral("2026-01-01"));
    settings.sync();

    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.bestSeconds, 42.0);
    QCOMPARE(r.bestNoflagSeconds, 0.0);
    QVERIFY(!r.bestNoflagDate.isValid());
}

void TestStats::testRecordNoflagBestZeroSecondsRejected()
{
    // Zero/negative durations are guard-railed: they never set the no-flag
    // best, even on a virgin record. Mirrors the recordWin sentinel for 0.
    QVERIFY(!Stats::recordNoflagBest(QStringLiteral("Beginner"), 0.0));
    QVERIFY(!Stats::recordNoflagBest(QStringLiteral("Beginner"), -1.0));
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.bestNoflagSeconds, 0.0);
    QVERIFY(!r.bestNoflagDate.isValid());
}

QTEST_MAIN(TestStats)
#include "tst_stats.moc"
