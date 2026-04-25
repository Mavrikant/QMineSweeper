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

    // win streak per difficulty
    void testStreakDefaultsZero();
    void testFirstWinSetsStreakOneAndBestOne();
    void testConsecutiveWinsExtendCurrentAndBest();
    void testLossResetsCurrentStreakOnly();
    void testWinAfterLossRestartsAtOne();
    void testStreakIsPerDifficulty();
    void testBestStreakDateStampedOnHighWaterOnly();
    void testResetWipesStreak();
    void testResetAllWipesStreak();
    void testLegacyRecordWithoutStreakLoadsAsZero();
    void testRecordWinReturnsOutcomeFields();

    // best partial-clear (per-difficulty hall of fame for unwon difficulties)
    void testBestSafePercentDefaultsZero();
    void testRecordLossDefaultArgsKeepBestSafePercentZero();
    void testRecordLossWithPercentSetsOnFirstCall();
    void testRecordLossWithHigherPercentBeats();
    void testRecordLossWithLowerPercentKeepsOriginal();
    void testRecordLossWithEqualPercentKeepsOriginalDate();
    void testRecordLossWithZeroPercentDoesNotMutate();
    void testRecordLossWithFullClearPercentBoundary();
    void testRecordLossWithOverflowPercentClampedTo100();
    void testRecordWinDoesNotTouchBestSafePercent();
    void testBestSafePercentIsPerDifficulty();
    void testResetWipesBestSafePercent();
    void testResetAllWipesBestSafePercent();
    void testLegacyRecordWithoutBestSafePercentLoadsAsZero();
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
    const auto outcome = Stats::recordWin(QStringLiteral("Beginner"), 15.5);
    QVERIFY(outcome.newRecord);
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.played, 1u);
    QCOMPARE(r.won, 1u);
    QCOMPARE(r.bestSeconds, 15.5);
}

void TestStats::testRecordWinSetsBestOnFirstWin()
{
    QVERIFY(Stats::recordWin(QStringLiteral("Beginner"), 30.0).newRecord);
    QCOMPARE(Stats::load(QStringLiteral("Beginner")).bestSeconds, 30.0);
}

void TestStats::testFasterWinBeatsBest()
{
    QVERIFY(Stats::recordWin(QStringLiteral("Beginner"), 30.0).newRecord);
    const auto nr = Stats::recordWin(QStringLiteral("Beginner"), 20.0);
    QVERIFY(nr.newRecord);
    QCOMPARE(Stats::load(QStringLiteral("Beginner")).bestSeconds, 20.0);
}

void TestStats::testSlowerWinKeepsBest()
{
    QVERIFY(Stats::recordWin(QStringLiteral("Beginner"), 20.0).newRecord);
    const auto nr = Stats::recordWin(QStringLiteral("Beginner"), 40.0);
    QVERIFY(!nr.newRecord);
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

void TestStats::testStreakDefaultsZero()
{
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.currentStreak, 0u);
    QCOMPARE(r.bestStreak, 0u);
    QVERIFY(!r.bestStreakDate.isValid());
}

void TestStats::testFirstWinSetsStreakOneAndBestOne()
{
    const QDate d{2026, 4, 25};
    const auto outcome = Stats::recordWin(QStringLiteral("Beginner"), 30.0, d);
    QCOMPARE(outcome.currentStreak, 1u);
    QVERIFY(outcome.newBestStreak);
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.currentStreak, 1u);
    QCOMPARE(r.bestStreak, 1u);
    QCOMPARE(r.bestStreakDate, d);
}

void TestStats::testConsecutiveWinsExtendCurrentAndBest()
{
    const QDate d1{2026, 4, 23};
    const QDate d2{2026, 4, 24};
    const QDate d3{2026, 4, 25};
    QVERIFY(Stats::recordWin(QStringLiteral("Beginner"), 30.0, d1).newBestStreak);
    const auto o2 = Stats::recordWin(QStringLiteral("Beginner"), 30.0, d2);
    QCOMPARE(o2.currentStreak, 2u);
    QVERIFY(o2.newBestStreak);
    const auto o3 = Stats::recordWin(QStringLiteral("Beginner"), 30.0, d3);
    QCOMPARE(o3.currentStreak, 3u);
    QVERIFY(o3.newBestStreak);
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.currentStreak, 3u);
    QCOMPARE(r.bestStreak, 3u);
    // Best-streak date is stamped on the most recent high-water-mark moment.
    QCOMPARE(r.bestStreakDate, d3);
}

void TestStats::testLossResetsCurrentStreakOnly()
{
    Stats::recordWin(QStringLiteral("Beginner"), 30.0, QDate{2026, 4, 23});
    Stats::recordWin(QStringLiteral("Beginner"), 30.0, QDate{2026, 4, 24});
    Stats::recordLoss(QStringLiteral("Beginner"));
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.currentStreak, 0u);
    QCOMPARE(r.bestStreak, 2u);
    QCOMPARE(r.bestStreakDate, (QDate{2026, 4, 24}));
}

void TestStats::testWinAfterLossRestartsAtOne()
{
    Stats::recordWin(QStringLiteral("Beginner"), 30.0, QDate{2026, 4, 23});
    Stats::recordWin(QStringLiteral("Beginner"), 30.0, QDate{2026, 4, 24});
    Stats::recordLoss(QStringLiteral("Beginner"));
    const auto outcome = Stats::recordWin(QStringLiteral("Beginner"), 30.0, QDate{2026, 4, 25});
    QCOMPARE(outcome.currentStreak, 1u);
    QVERIFY(!outcome.newBestStreak);
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.currentStreak, 1u);
    QCOMPARE(r.bestStreak, 2u);
    // Best-streak date stays pinned on the original high-water moment.
    QCOMPARE(r.bestStreakDate, (QDate{2026, 4, 24}));
}

void TestStats::testStreakIsPerDifficulty()
{
    Stats::recordWin(QStringLiteral("Beginner"), 30.0, QDate{2026, 4, 23});
    Stats::recordWin(QStringLiteral("Beginner"), 30.0, QDate{2026, 4, 23});
    // A loss on Expert must not touch Beginner's streak.
    Stats::recordLoss(QStringLiteral("Expert"));
    const auto bn = Stats::load(QStringLiteral("Beginner"));
    const auto ex = Stats::load(QStringLiteral("Expert"));
    QCOMPARE(bn.currentStreak, 2u);
    QCOMPARE(bn.bestStreak, 2u);
    QCOMPARE(ex.currentStreak, 0u);
    QCOMPARE(ex.bestStreak, 0u);
}

void TestStats::testBestStreakDateStampedOnHighWaterOnly()
{
    // Win streak goes 1 → 2 → 3 → loss → 1 (no high-water bump) → 2 (still
    // no bump, ties old best of 3 only after one more) → 3 (ties, no bump
    // because strict greater-than) → 4 (new high-water, date stamped).
    Stats::recordWin(QStringLiteral("Beginner"), 30.0, QDate{2026, 4, 1});
    Stats::recordWin(QStringLiteral("Beginner"), 30.0, QDate{2026, 4, 2});
    Stats::recordWin(QStringLiteral("Beginner"), 30.0, QDate{2026, 4, 3});
    Stats::recordLoss(QStringLiteral("Beginner"));
    Stats::recordWin(QStringLiteral("Beginner"), 30.0, QDate{2026, 4, 5});                         // current=1, best stays 3
    Stats::recordWin(QStringLiteral("Beginner"), 30.0, QDate{2026, 4, 6});                         // current=2, best stays 3
    const auto tieOutcome = Stats::recordWin(QStringLiteral("Beginner"), 30.0, QDate{2026, 4, 7}); // current=3, ties best
    QVERIFY(!tieOutcome.newBestStreak);
    const auto r1 = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r1.bestStreak, 3u);
    QCOMPARE(r1.bestStreakDate, (QDate{2026, 4, 3}));                                               // pinned on first time it hit 3
    const auto bumpOutcome = Stats::recordWin(QStringLiteral("Beginner"), 30.0, QDate{2026, 4, 8}); // current=4, new best
    QVERIFY(bumpOutcome.newBestStreak);
    const auto r2 = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r2.bestStreak, 4u);
    QCOMPARE(r2.bestStreakDate, (QDate{2026, 4, 8}));
}

void TestStats::testResetWipesStreak()
{
    Stats::recordWin(QStringLiteral("Beginner"), 30.0, QDate{2026, 4, 25});
    Stats::recordWin(QStringLiteral("Beginner"), 30.0, QDate{2026, 4, 25});
    Stats::reset(QStringLiteral("Beginner"));
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.currentStreak, 0u);
    QCOMPARE(r.bestStreak, 0u);
    QVERIFY(!r.bestStreakDate.isValid());
}

void TestStats::testResetAllWipesStreak()
{
    Stats::recordWin(QStringLiteral("Beginner"), 30.0, QDate{2026, 4, 25});
    Stats::recordWin(QStringLiteral("Expert"), 200.0, QDate{2026, 4, 25});
    Stats::resetAll();
    QCOMPARE(Stats::load(QStringLiteral("Beginner")).currentStreak, 0u);
    QCOMPARE(Stats::load(QStringLiteral("Beginner")).bestStreak, 0u);
    QCOMPARE(Stats::load(QStringLiteral("Expert")).currentStreak, 0u);
    QCOMPARE(Stats::load(QStringLiteral("Expert")).bestStreak, 0u);
}

void TestStats::testLegacyRecordWithoutStreakLoadsAsZero()
{
    // Pre-1.16 record: no streak_* keys present — must load as 0/0/invalid.
    QSettings settings;
    settings.setValue(QStringLiteral("stats/Beginner/played"), 5u);
    settings.setValue(QStringLiteral("stats/Beginner/won"), 3u);
    settings.setValue(QStringLiteral("stats/Beginner/best_seconds"), 42.0);
    settings.setValue(QStringLiteral("stats/Beginner/best_date"), QStringLiteral("2026-01-01"));
    settings.sync();

    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.played, 5u);
    QCOMPARE(r.currentStreak, 0u);
    QCOMPARE(r.bestStreak, 0u);
    QVERIFY(!r.bestStreakDate.isValid());
}

void TestStats::testRecordWinReturnsOutcomeFields()
{
    // Sanity that the WinOutcome carries newRecord, currentStreak, and
    // newBestStreak together — the production caller reads all three.
    const auto first = Stats::recordWin(QStringLiteral("Beginner"), 30.0);
    QVERIFY(first.newRecord);
    QCOMPARE(first.currentStreak, 1u);
    QVERIFY(first.newBestStreak);
    const auto slower = Stats::recordWin(QStringLiteral("Beginner"), 60.0);
    QVERIFY(!slower.newRecord);
    QCOMPARE(slower.currentStreak, 2u);
    QVERIFY(slower.newBestStreak);
}

void TestStats::testBestSafePercentDefaultsZero()
{
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.bestSafePercent, 0u);
    QVERIFY(!r.bestSafePercentDate.isValid());
}

void TestStats::testRecordLossDefaultArgsKeepBestSafePercentZero()
{
    // Default-arg recordLoss (no percent supplied) must not touch the
    // partial-clear best — preserves source-compat for callers that don't
    // care about the new field (e.g. legacy tests).
    Stats::recordLoss(QStringLiteral("Beginner"));
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.played, 1u);
    QCOMPARE(r.bestSafePercent, 0u);
    QVERIFY(!r.bestSafePercentDate.isValid());
}

void TestStats::testRecordLossWithPercentSetsOnFirstCall()
{
    const QDate d{2026, 4, 25};
    Stats::recordLoss(QStringLiteral("Beginner"), 47, d);
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.played, 1u);
    QCOMPARE(r.bestSafePercent, 47u);
    QCOMPARE(r.bestSafePercentDate, d);
}

void TestStats::testRecordLossWithHigherPercentBeats()
{
    const QDate d1{2026, 1, 1};
    const QDate d2{2026, 4, 25};
    Stats::recordLoss(QStringLiteral("Expert"), 30, d1);
    Stats::recordLoss(QStringLiteral("Expert"), 70, d2);
    const auto r = Stats::load(QStringLiteral("Expert"));
    QCOMPARE(r.bestSafePercent, 70u);
    QCOMPARE(r.bestSafePercentDate, d2);
}

void TestStats::testRecordLossWithLowerPercentKeepsOriginal()
{
    const QDate d1{2026, 1, 1};
    const QDate d2{2026, 4, 25};
    Stats::recordLoss(QStringLiteral("Expert"), 70, d1);
    Stats::recordLoss(QStringLiteral("Expert"), 50, d2);
    const auto r = Stats::load(QStringLiteral("Expert"));
    QCOMPARE(r.bestSafePercent, 70u);
    QCOMPARE(r.bestSafePercentDate, d1);
}

void TestStats::testRecordLossWithEqualPercentKeepsOriginalDate()
{
    // Strict greater-than semantics: ties don't bump the date — mirrors the
    // best-streak date convention in testBestStreakDateStampedOnHighWaterOnly.
    const QDate d1{2026, 1, 1};
    const QDate d2{2026, 4, 25};
    Stats::recordLoss(QStringLiteral("Expert"), 60, d1);
    Stats::recordLoss(QStringLiteral("Expert"), 60, d2);
    const auto r = Stats::load(QStringLiteral("Expert"));
    QCOMPARE(r.bestSafePercent, 60u);
    QCOMPARE(r.bestSafePercentDate, d1);
}

void TestStats::testRecordLossWithZeroPercentDoesNotMutate()
{
    // First seed a non-zero best, then a zero-percent loss must leave it.
    Stats::recordLoss(QStringLiteral("Expert"), 40, QDate{2026, 1, 1});
    Stats::recordLoss(QStringLiteral("Expert"), 0, QDate{2026, 4, 25});
    const auto r = Stats::load(QStringLiteral("Expert"));
    QCOMPARE(r.played, 2u);
    QCOMPARE(r.bestSafePercent, 40u);
    QCOMPARE(r.bestSafePercentDate, (QDate{2026, 1, 1}));
}

void TestStats::testRecordLossWithFullClearPercentBoundary()
{
    // 100 is a defensible boundary value: a player who cleared every safe
    // cell but stepped on a mine on a chord click after the last open. Not
    // reachable in production (last-cell open triggers win first) but the
    // recording layer must accept it without clamping.
    Stats::recordLoss(QStringLiteral("Expert"), 100, QDate{2026, 4, 25});
    const auto r = Stats::load(QStringLiteral("Expert"));
    QCOMPARE(r.bestSafePercent, 100u);
}

void TestStats::testRecordLossWithOverflowPercentClampedTo100()
{
    // Defence-in-depth: a future caller that passes an overflowed value
    // (e.g. 200 from an arithmetic bug) must still produce a sane record.
    Stats::recordLoss(QStringLiteral("Expert"), 200, QDate{2026, 4, 25});
    const auto r = Stats::load(QStringLiteral("Expert"));
    QCOMPARE(r.bestSafePercent, 100u);
}

void TestStats::testRecordWinDoesNotTouchBestSafePercent()
{
    // Seed a partial-clear best, then win — recordWin must not zero or
    // overwrite the partial-clear field. Persistence semantics: the
    // partial-clear stays in QSettings; the dialog hides it once a win
    // exists.
    Stats::recordLoss(QStringLiteral("Expert"), 60, QDate{2026, 1, 1});
    QVERIFY(Stats::recordWin(QStringLiteral("Expert"), 250.0, QDate{2026, 4, 25}));
    const auto r = Stats::load(QStringLiteral("Expert"));
    QCOMPARE(r.bestSafePercent, 60u);
    QCOMPARE(r.bestSafePercentDate, (QDate{2026, 1, 1}));
    QCOMPARE(r.won, 1u);
}

void TestStats::testBestSafePercentIsPerDifficulty()
{
    Stats::recordLoss(QStringLiteral("Beginner"), 80, QDate{2026, 4, 25});
    Stats::recordLoss(QStringLiteral("Expert"), 40, QDate{2026, 4, 25});
    QCOMPARE(Stats::load(QStringLiteral("Beginner")).bestSafePercent, 80u);
    QCOMPARE(Stats::load(QStringLiteral("Intermediate")).bestSafePercent, 0u);
    QCOMPARE(Stats::load(QStringLiteral("Expert")).bestSafePercent, 40u);
}

void TestStats::testResetWipesBestSafePercent()
{
    Stats::recordLoss(QStringLiteral("Expert"), 60, QDate{2026, 4, 25});
    Stats::reset(QStringLiteral("Expert"));
    const auto r = Stats::load(QStringLiteral("Expert"));
    QCOMPARE(r.bestSafePercent, 0u);
    QVERIFY(!r.bestSafePercentDate.isValid());
}

void TestStats::testResetAllWipesBestSafePercent()
{
    Stats::recordLoss(QStringLiteral("Beginner"), 80, QDate{2026, 4, 25});
    Stats::recordLoss(QStringLiteral("Expert"), 60, QDate{2026, 4, 25});
    Stats::resetAll();
    QCOMPARE(Stats::load(QStringLiteral("Beginner")).bestSafePercent, 0u);
    QCOMPARE(Stats::load(QStringLiteral("Expert")).bestSafePercent, 0u);
}

void TestStats::testLegacyRecordWithoutBestSafePercentLoadsAsZero()
{
    // Pre-1.28 record: no best_safe_percent_* keys present — must load as
    // 0 / invalid date so an upgrading user's first loss after the upgrade
    // sets the record cleanly.
    QSettings settings;
    settings.setValue(QStringLiteral("stats/Beginner/played"), 5u);
    settings.setValue(QStringLiteral("stats/Beginner/won"), 3u);
    settings.setValue(QStringLiteral("stats/Beginner/best_seconds"), 42.0);
    settings.setValue(QStringLiteral("stats/Beginner/best_date"), QStringLiteral("2026-01-01"));
    settings.sync();

    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.played, 5u);
    QCOMPARE(r.bestSafePercent, 0u);
    QVERIFY(!r.bestSafePercentDate.isValid());
}

QTEST_MAIN(TestStats)
#include "tst_stats.moc"
