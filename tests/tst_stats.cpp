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

    // LossOutcome — `🎯 New best %!` flair gate
    void testLossOutcomeDefaultArgsReturnsNoNewBest();
    void testLossOutcomeWithZeroPercentReturnsNoNewBest();
    void testLossOutcomeFirstPositivePercentReturnsNewBest();
    void testLossOutcomeHigherPercentReturnsNewBest();
    void testLossOutcomeEqualPercentReturnsNoNewBest();
    void testLossOutcomeLowerPercentReturnsNoNewBest();
    void testLossOutcomeOverflowAtCapReturnsNoNewBest();
    void testLossOutcomeBoolConversion();

    // best 3BV/s + WinOutcome.newBestBvPerSecond — `⚡ New best 3BV/s!` flair
    void testBestBvPerSecondDefaultsZero();
    void testRecordWinDefaultArgsKeepsBestBvPerSecondZero();
    void testRecordWinWithBvRateSetsOnFirstCall();
    void testRecordWinWithHigherBvRateBeats();
    void testRecordWinWithLowerBvRateKeepsOriginal();
    void testRecordWinWithEqualBvRateKeepsOriginalDate();
    void testRecordWinWithZeroBvRateNoOp();
    void testRecordWinWithNegativeBvRateNoOp();
    void testRecordLossDoesNotTouchBestBvPerSecond();
    void testBestBvPerSecondIsPerDifficulty();
    void testResetWipesBestBvPerSecond();
    void testResetAllWipesBestBvPerSecond();
    void testLegacyRecordWithoutBestBvPerSecondLoadsAsZero();
    void testFasterClockMayNotBeatBvRateAndViceVersa();
    void testWinOutcomeBvRateIndependentOfNewRecord();

    // best flag accuracy + LossOutcome.newBestFlagAccuracyPercent
    void testBestFlagAccuracyDefaultsZero();
    void testRecordLossDefaultArgsKeepsBestFlagAccuracyZero();
    void testRecordLossWithFlagAccuracySetsOnFirstCall();
    void testRecordLossWithHigherFlagAccuracyBeats();
    void testRecordLossWithLowerFlagAccuracyKeepsOriginal();
    void testRecordLossWithEqualFlagAccuracyKeepsOriginalDate();
    void testRecordLossWithZeroFlagAccuracyDoesNotMutate();
    void testRecordLossWithFullFlagAccuracyBoundary();
    void testRecordLossWithOverflowFlagAccuracyClampedTo100();
    void testRecordWinDoesNotTouchBestFlagAccuracy();
    void testBestFlagAccuracyIsPerDifficulty();
    void testResetWipesBestFlagAccuracy();
    void testResetAllWipesBestFlagAccuracy();
    void testLegacyRecordWithoutBestFlagAccuracyLoadsAsZero();
    void testLossOutcomeFlagAccuracyIndependentOfSafePercent();
    void testLossOutcomeBoolStillTracksOnlySafePercent();

    // total_seconds_won accumulator + WinOutcome.winsAfter / averageSecondsAfter
    void testTotalSecondsWonDefaultsZero();
    void testRecordWinAccumulatesTotalSeconds();
    void testMultipleWinsAccumulateTotalSeconds();
    void testRecordLossDoesNotTouchTotalSecondsWon();
    void testRecordWinZeroSecondsDoesNotAccumulate();
    void testWinOutcomeWinsAfterIsPostIncrement();
    void testWinOutcomeAverageSecondsAfterEqualsMean();
    void testWinOutcomeAverageZeroOnSubTickFirstWin();
    void testTotalSecondsWonIsPerDifficulty();
    void testResetWipesTotalSecondsWon();
    void testResetAllWipesTotalSecondsWon();
    void testLegacyRecordWithoutTotalSecondsWonLoadsAsZero();

    // WinOutcome.bestSecondsAfter — drives the win-dialog
    // "Average: %1 (best %2)" companion suffix
    void testWinOutcomeDefaultBestSecondsAfterIsZero();
    void testWinOutcomeBestSecondsAfterFirstWinEqualsSeconds();
    void testWinOutcomeBestSecondsAfterTracksFasterWin();
    void testWinOutcomeBestSecondsAfterKeepsPriorOnSlowerWin();
    void testWinOutcomeBestSecondsAfterSubTickWinKeepsPrior();
    void testWinOutcomeBestSecondsAfterFirstWinSubTickStaysZero();
    void testWinOutcomeBestSecondsAfterMatchesPersisted();

    // last_win_date — drives the loss-dialog "Last win: %1" line
    void testLastWinDateDefaultsInvalid();
    void testFirstWinStampsLastWinDate();
    void testSlowerWinOverwritesLastWinDateEvenWhenBestUnchanged();
    void testFasterWinAlsoOverwritesLastWinDate();
    void testLossDoesNotTouchLastWinDate();
    void testSubTickWinStampsLastWinDate();
    void testLastWinDateIsPerDifficulty();
    void testResetWipesLastWinDate();
    void testResetAllWipesLastWinDate();
    void testLegacyRecordWithoutLastWinDateLoadsAsInvalid();
    void testLegacyRecordWithWonButNoLastWinDateLoadsAsInvalid();

    // Loss-dialog "Average: %1 (best %2)" lifetime-mean line — pin the
    // loaded-record contract the loss-side render gate reads. The win-side gate
    // is already pinned via WinOutcome.{winsAfter,averageSecondsAfter,bestSecondsAfter};
    // the loss path doesn't have a recordLoss-returned outcome to thread the
    // mean, so it computes from `Stats::load(diff).{won,totalSecondsWon,bestSeconds}`
    // directly. These tests pin that contract explicitly.
    void testLoadAfterThreeWinsExposesAverageForLossDialog();
    void testLoadAfterThreeWinsExposesBestForLossDialog();
    void testLoadAfterTwoWinsBelowLossDialogGate();
    void testLoadAfterAllSubTickWinsLeavesLossDialogGateClosed();
    void testLoadAfterMixedSubTickAndRealWinsForLossDialog();
    void testLoadAfterLossDoesNotDisturbAverageForLossDialog();

    // Win-dialog "✨ Beat your average!" flair gate — pin the comparison
    // arithmetic the gate consumes. The flair fires iff
    //   `winAverageSeconds > 0.0 && seconds > 0.0 && seconds < winAverageSeconds`,
    // where `winAverageSeconds = (winsAfter >= 3) ? averageSecondsAfter : 0.0`
    // and `seconds` is the just-finished run's duration. Mutually exclusive
    // with `🏆 New record!` (caller-side `else if`); these tests pin the
    // data-side contract that drives the gate.
    void testBeatAverageGateFiresWhenSecondsBelowMean();
    void testBeatAverageGateClosedWhenSecondsAtMean();
    void testBeatAverageGateClosedWhenSecondsAboveMean();
    void testBeatAverageGateRequiresThreeWinsCallerSide();
    void testBeatAverageGateNotPoisonedBySubTickWinsAfterRealWins();
    void testBeatAverageGateNotMutatedByLoss();

    // LossOutcome.priorStreak — drives the loss-dialog "💔 Streak ended at %1" line
    void testLossOutcomeDefaultPriorStreakIsZero();
    void testFirstEverLossPriorStreakIsZero();
    void testLossAfterOneWinPriorStreakIsOne();
    void testLossAfterFiveWinsPriorStreakIsFive();
    void testTwoConsecutiveLossesSecondPriorStreakIsZero();
    void testWinThenLossThenWinThenLossPriorStreakResets();
    void testPriorStreakIsPerDifficulty();
    void testRecordLossZerosCurrentStreakRegardlessOfPriorValue();
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
    Stats::recordLoss(QStringLiteral("Beginner"), 47, 0, d);
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.played, 1u);
    QCOMPARE(r.bestSafePercent, 47u);
    QCOMPARE(r.bestSafePercentDate, d);
}

void TestStats::testRecordLossWithHigherPercentBeats()
{
    const QDate d1{2026, 1, 1};
    const QDate d2{2026, 4, 25};
    Stats::recordLoss(QStringLiteral("Expert"), 30, 0, d1);
    Stats::recordLoss(QStringLiteral("Expert"), 70, 0, d2);
    const auto r = Stats::load(QStringLiteral("Expert"));
    QCOMPARE(r.bestSafePercent, 70u);
    QCOMPARE(r.bestSafePercentDate, d2);
}

void TestStats::testRecordLossWithLowerPercentKeepsOriginal()
{
    const QDate d1{2026, 1, 1};
    const QDate d2{2026, 4, 25};
    Stats::recordLoss(QStringLiteral("Expert"), 70, 0, d1);
    Stats::recordLoss(QStringLiteral("Expert"), 50, 0, d2);
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
    Stats::recordLoss(QStringLiteral("Expert"), 60, 0, d1);
    Stats::recordLoss(QStringLiteral("Expert"), 60, 0, d2);
    const auto r = Stats::load(QStringLiteral("Expert"));
    QCOMPARE(r.bestSafePercent, 60u);
    QCOMPARE(r.bestSafePercentDate, d1);
}

void TestStats::testRecordLossWithZeroPercentDoesNotMutate()
{
    // First seed a non-zero best, then a zero-percent loss must leave it.
    Stats::recordLoss(QStringLiteral("Expert"), 40, 0, QDate{2026, 1, 1});
    Stats::recordLoss(QStringLiteral("Expert"), 0, 0, QDate{2026, 4, 25});
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
    Stats::recordLoss(QStringLiteral("Expert"), 100, 0, QDate{2026, 4, 25});
    const auto r = Stats::load(QStringLiteral("Expert"));
    QCOMPARE(r.bestSafePercent, 100u);
}

void TestStats::testRecordLossWithOverflowPercentClampedTo100()
{
    // Defence-in-depth: a future caller that passes an overflowed value
    // (e.g. 200 from an arithmetic bug) must still produce a sane record.
    Stats::recordLoss(QStringLiteral("Expert"), 200, 0, QDate{2026, 4, 25});
    const auto r = Stats::load(QStringLiteral("Expert"));
    QCOMPARE(r.bestSafePercent, 100u);
}

void TestStats::testRecordWinDoesNotTouchBestSafePercent()
{
    // Seed a partial-clear best, then win — recordWin must not zero or
    // overwrite the partial-clear field. Persistence semantics: the
    // partial-clear stays in QSettings; the dialog hides it once a win
    // exists.
    Stats::recordLoss(QStringLiteral("Expert"), 60, 0, QDate{2026, 1, 1});
    QVERIFY(Stats::recordWin(QStringLiteral("Expert"), 250.0, QDate{2026, 4, 25}));
    const auto r = Stats::load(QStringLiteral("Expert"));
    QCOMPARE(r.bestSafePercent, 60u);
    QCOMPARE(r.bestSafePercentDate, (QDate{2026, 1, 1}));
    QCOMPARE(r.won, 1u);
}

void TestStats::testBestSafePercentIsPerDifficulty()
{
    Stats::recordLoss(QStringLiteral("Beginner"), 80, 0, QDate{2026, 4, 25});
    Stats::recordLoss(QStringLiteral("Expert"), 40, 0, QDate{2026, 4, 25});
    QCOMPARE(Stats::load(QStringLiteral("Beginner")).bestSafePercent, 80u);
    QCOMPARE(Stats::load(QStringLiteral("Intermediate")).bestSafePercent, 0u);
    QCOMPARE(Stats::load(QStringLiteral("Expert")).bestSafePercent, 40u);
}

void TestStats::testResetWipesBestSafePercent()
{
    Stats::recordLoss(QStringLiteral("Expert"), 60, 0, QDate{2026, 4, 25});
    Stats::reset(QStringLiteral("Expert"));
    const auto r = Stats::load(QStringLiteral("Expert"));
    QCOMPARE(r.bestSafePercent, 0u);
    QVERIFY(!r.bestSafePercentDate.isValid());
}

void TestStats::testResetAllWipesBestSafePercent()
{
    Stats::recordLoss(QStringLiteral("Beginner"), 80, 0, QDate{2026, 4, 25});
    Stats::recordLoss(QStringLiteral("Expert"), 60, 0, QDate{2026, 4, 25});
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

void TestStats::testLossOutcomeDefaultArgsReturnsNoNewBest()
{
    // Default-arg recordLoss (no percent supplied) cannot set a new best —
    // safePercent==0 short-circuits the update path.
    const auto out = Stats::recordLoss(QStringLiteral("Beginner"));
    QVERIFY(!out.newBestSafePercent);
    QVERIFY(!static_cast<bool>(out));
}

void TestStats::testLossOutcomeWithZeroPercentReturnsNoNewBest()
{
    // Explicit 0 percent (e.g. first-click boom) must never set the flair —
    // mirrors the recordLoss zero-sentinel: no record persisted, no flair.
    const auto out = Stats::recordLoss(QStringLiteral("Beginner"), 0, 0, QDate{2026, 4, 25});
    QVERIFY(!out.newBestSafePercent);
}

void TestStats::testLossOutcomeFirstPositivePercentReturnsNewBest()
{
    // The very first loss with safePercent > 0 transitions the field
    // 0 → some positive value, which IS a new high-water mark — the flair
    // must fire (parallel to the first-win == newRecord case).
    const auto out = Stats::recordLoss(QStringLiteral("Beginner"), 47, 0, QDate{2026, 4, 25});
    QVERIFY(out.newBestSafePercent);
    QVERIFY(static_cast<bool>(out));
}

void TestStats::testLossOutcomeHigherPercentReturnsNewBest()
{
    // A subsequent loss that strictly beats the prior best fires the flair.
    Stats::recordLoss(QStringLiteral("Expert"), 30, 0, QDate{2026, 1, 1});
    const auto out = Stats::recordLoss(QStringLiteral("Expert"), 70, 0, QDate{2026, 4, 25});
    QVERIFY(out.newBestSafePercent);
}

void TestStats::testLossOutcomeEqualPercentReturnsNoNewBest()
{
    // Strict greater-than semantics: a tie with the prior best does NOT fire
    // the flair (matches the persistence layer's date-pin behaviour).
    Stats::recordLoss(QStringLiteral("Expert"), 60, 0, QDate{2026, 1, 1});
    const auto out = Stats::recordLoss(QStringLiteral("Expert"), 60, 0, QDate{2026, 4, 25});
    QVERIFY(!out.newBestSafePercent);
}

void TestStats::testLossOutcomeLowerPercentReturnsNoNewBest()
{
    // A worse loss leaves the prior record intact — no flair.
    Stats::recordLoss(QStringLiteral("Expert"), 70, 0, QDate{2026, 1, 1});
    const auto out = Stats::recordLoss(QStringLiteral("Expert"), 50, 0, QDate{2026, 4, 25});
    QVERIFY(!out.newBestSafePercent);
}

void TestStats::testLossOutcomeOverflowAtCapReturnsNoNewBest()
{
    // Defence-in-depth: an overflowed value clamps to 100 in the persisted
    // record. Once a record has been clamped to 100, a subsequent overflow
    // call is a tie, NOT a new best — so the flair must not fire twice.
    Stats::recordLoss(QStringLiteral("Expert"), 200, 0, QDate{2026, 1, 1});
    const auto out = Stats::recordLoss(QStringLiteral("Expert"), 250, 0, QDate{2026, 4, 25});
    QVERIFY(!out.newBestSafePercent);
}

void TestStats::testLossOutcomeBoolConversion()
{
    // The explicit operator bool exists so future call sites can do
    // `if (Stats::recordLoss(...))`. Confirm it tracks newBestSafePercent.
    const auto first = Stats::recordLoss(QStringLiteral("Beginner"), 30, 0, QDate{2026, 4, 25});
    QVERIFY(static_cast<bool>(first));
    const auto tie = Stats::recordLoss(QStringLiteral("Beginner"), 30, 0, QDate{2026, 4, 25});
    QVERIFY(!static_cast<bool>(tie));
}

void TestStats::testBestBvPerSecondDefaultsZero()
{
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.bestBvPerSecond, 0.0);
    QVERIFY(!r.bestBvPerSecondDate.isValid());
}

void TestStats::testRecordWinDefaultArgsKeepsBestBvPerSecondZero()
{
    // Default-arg recordWin (no bvPerSecond supplied) must not touch the
    // 3BV/s best — preserves source-compat for the 17 pre-1.30 test sites
    // that don't care about the new field.
    const auto out = Stats::recordWin(QStringLiteral("Beginner"), 30.0);
    QVERIFY(!out.newBestBvPerSecond);
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.bestBvPerSecond, 0.0);
    QVERIFY(!r.bestBvPerSecondDate.isValid());
}

void TestStats::testRecordWinWithBvRateSetsOnFirstCall()
{
    const QDate d{2026, 4, 25};
    const auto out = Stats::recordWin(QStringLiteral("Beginner"), 30.0, d, 2.5);
    QVERIFY(out.newBestBvPerSecond);
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.bestBvPerSecond, 2.5);
    QCOMPARE(r.bestBvPerSecondDate, d);
}

void TestStats::testRecordWinWithHigherBvRateBeats()
{
    const QDate d1{2026, 1, 1};
    const QDate d2{2026, 4, 25};
    QVERIFY(Stats::recordWin(QStringLiteral("Expert"), 250.0, d1, 1.20).newBestBvPerSecond);
    const auto out = Stats::recordWin(QStringLiteral("Expert"), 240.0, d2, 1.55);
    QVERIFY(out.newBestBvPerSecond);
    const auto r = Stats::load(QStringLiteral("Expert"));
    QCOMPARE(r.bestBvPerSecond, 1.55);
    QCOMPARE(r.bestBvPerSecondDate, d2);
}

void TestStats::testRecordWinWithLowerBvRateKeepsOriginal()
{
    const QDate d1{2026, 1, 1};
    const QDate d2{2026, 4, 25};
    QVERIFY(Stats::recordWin(QStringLiteral("Expert"), 200.0, d1, 1.80).newBestBvPerSecond);
    const auto out = Stats::recordWin(QStringLiteral("Expert"), 220.0, d2, 1.40);
    QVERIFY(!out.newBestBvPerSecond);
    const auto r = Stats::load(QStringLiteral("Expert"));
    QCOMPARE(r.bestBvPerSecond, 1.80);
    QCOMPARE(r.bestBvPerSecondDate, d1);
}

void TestStats::testRecordWinWithEqualBvRateKeepsOriginalDate()
{
    // Strict greater-than semantics: ties don't bump the date — mirrors the
    // best-streak / best-percent date-pin convention. In production, FP
    // rounding makes ties effectively impossible, but the semantics matter
    // for fixed-layout test setups.
    const QDate d1{2026, 1, 1};
    const QDate d2{2026, 4, 25};
    QVERIFY(Stats::recordWin(QStringLiteral("Expert"), 200.0, d1, 1.80).newBestBvPerSecond);
    const auto out = Stats::recordWin(QStringLiteral("Expert"), 200.0, d2, 1.80);
    QVERIFY(!out.newBestBvPerSecond);
    const auto r = Stats::load(QStringLiteral("Expert"));
    QCOMPARE(r.bestBvPerSecond, 1.80);
    QCOMPARE(r.bestBvPerSecondDate, d1);
}

void TestStats::testRecordWinWithZeroBvRateNoOp()
{
    // First seed a non-zero best, then a zero-bvRate win must leave it.
    // Mirrors the bestSeconds 0.0-sentinel — a sub-tick win (only reachable
    // from setFixedLayout-driven test setups) skips the update path.
    QVERIFY(Stats::recordWin(QStringLiteral("Expert"), 200.0, QDate{2026, 1, 1}, 1.40).newBestBvPerSecond);
    const auto out = Stats::recordWin(QStringLiteral("Expert"), 220.0, QDate{2026, 4, 25}, 0.0);
    QVERIFY(!out.newBestBvPerSecond);
    const auto r = Stats::load(QStringLiteral("Expert"));
    QCOMPARE(r.played, 2u);
    QCOMPARE(r.bestBvPerSecond, 1.40);
    QCOMPARE(r.bestBvPerSecondDate, (QDate{2026, 1, 1}));
}

void TestStats::testRecordWinWithNegativeBvRateNoOp()
{
    // Defence-in-depth: a future caller that passes a pathologically negative
    // bvRate (arithmetic bug, sign flip) must not set the record. Mirrors the
    // recordNoflagBest negative-second guard.
    const auto out = Stats::recordWin(QStringLiteral("Beginner"), 30.0, QDate{2026, 4, 25}, -1.5);
    QVERIFY(!out.newBestBvPerSecond);
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.bestBvPerSecond, 0.0);
    QVERIFY(!r.bestBvPerSecondDate.isValid());
}

void TestStats::testRecordLossDoesNotTouchBestBvPerSecond()
{
    // Seed a 3BV/s best, then lose. recordLoss must not zero or overwrite the
    // 3BV/s field — losses are accounted on a different axis.
    QVERIFY(Stats::recordWin(QStringLiteral("Expert"), 200.0, QDate{2026, 1, 1}, 1.55).newBestBvPerSecond);
    Stats::recordLoss(QStringLiteral("Expert"), 60, 0, QDate{2026, 4, 25});
    const auto r = Stats::load(QStringLiteral("Expert"));
    QCOMPARE(r.bestBvPerSecond, 1.55);
    QCOMPARE(r.bestBvPerSecondDate, (QDate{2026, 1, 1}));
}

void TestStats::testBestBvPerSecondIsPerDifficulty()
{
    Stats::recordWin(QStringLiteral("Beginner"), 15.0, QDate{2026, 4, 25}, 3.20);
    Stats::recordWin(QStringLiteral("Expert"), 200.0, QDate{2026, 4, 25}, 1.55);
    QCOMPARE(Stats::load(QStringLiteral("Beginner")).bestBvPerSecond, 3.20);
    QCOMPARE(Stats::load(QStringLiteral("Intermediate")).bestBvPerSecond, 0.0);
    QCOMPARE(Stats::load(QStringLiteral("Expert")).bestBvPerSecond, 1.55);
}

void TestStats::testResetWipesBestBvPerSecond()
{
    QVERIFY(Stats::recordWin(QStringLiteral("Expert"), 200.0, QDate{2026, 4, 25}, 1.55).newBestBvPerSecond);
    Stats::reset(QStringLiteral("Expert"));
    const auto r = Stats::load(QStringLiteral("Expert"));
    QCOMPARE(r.bestBvPerSecond, 0.0);
    QVERIFY(!r.bestBvPerSecondDate.isValid());
}

void TestStats::testResetAllWipesBestBvPerSecond()
{
    Stats::recordWin(QStringLiteral("Beginner"), 15.0, QDate{2026, 4, 25}, 3.20);
    Stats::recordWin(QStringLiteral("Expert"), 200.0, QDate{2026, 4, 25}, 1.55);
    Stats::resetAll();
    QCOMPARE(Stats::load(QStringLiteral("Beginner")).bestBvPerSecond, 0.0);
    QCOMPARE(Stats::load(QStringLiteral("Expert")).bestBvPerSecond, 0.0);
}

void TestStats::testLegacyRecordWithoutBestBvPerSecondLoadsAsZero()
{
    // Pre-1.30 record: no best_bv_per_second_* keys present — must load as
    // 0.0 / invalid date so an upgrading user's first 1.30.0 win seeds the
    // record cleanly.
    QSettings settings;
    settings.setValue(QStringLiteral("stats/Beginner/played"), 5u);
    settings.setValue(QStringLiteral("stats/Beginner/won"), 3u);
    settings.setValue(QStringLiteral("stats/Beginner/best_seconds"), 42.0);
    settings.setValue(QStringLiteral("stats/Beginner/best_date"), QStringLiteral("2026-01-01"));
    settings.sync();

    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.played, 5u);
    QCOMPARE(r.bestSeconds, 42.0);
    QCOMPARE(r.bestBvPerSecond, 0.0);
    QVERIFY(!r.bestBvPerSecondDate.isValid());
}

void TestStats::testFasterClockMayNotBeatBvRateAndViceVersa()
{
    // Independence pin: best-time and best-3BV/s axes are not coupled. A
    // faster clock on a smaller-3BV board can leave 3BV/s best untouched;
    // a slower clock on a denser-3BV board can set 3BV/s without beating
    // the clock.
    const QDate d1{2026, 4, 23};
    const QDate d2{2026, 4, 24};
    const QDate d3{2026, 4, 25};

    // First run: 30s, 1.5 3BV/s on Beginner. Both axes set.
    {
        const auto o = Stats::recordWin(QStringLiteral("Beginner"), 30.0, d1, 1.50);
        QVERIFY(o.newRecord);
        QVERIFY(o.newBestBvPerSecond);
    }
    // Second run: 25s (faster clock, new bestSeconds) but only 1.20 3BV/s
    // (slower rate — easier board). Best-time updates; 3BV/s does not.
    {
        const auto o = Stats::recordWin(QStringLiteral("Beginner"), 25.0, d2, 1.20);
        QVERIFY(o.newRecord);
        QVERIFY(!o.newBestBvPerSecond);
    }
    // Third run: 28s (slower than 25, no clock record) but 2.10 3BV/s
    // (denser board, faster rate). 3BV/s updates; clock does not.
    {
        const auto o = Stats::recordWin(QStringLiteral("Beginner"), 28.0, d3, 2.10);
        QVERIFY(!o.newRecord);
        QVERIFY(o.newBestBvPerSecond);
    }
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.bestSeconds, 25.0);
    QCOMPARE(r.bestDate, d2);
    QCOMPARE(r.bestBvPerSecond, 2.10);
    QCOMPARE(r.bestBvPerSecondDate, d3);
}

void TestStats::testWinOutcomeBvRateIndependentOfNewRecord()
{
    // The four WinOutcome flags carry independent signal. Pin the matrix so
    // a future refactor can't accidentally couple them.
    const auto first = Stats::recordWin(QStringLiteral("Beginner"), 30.0, QDate{2026, 4, 23}, 1.50);
    QVERIFY(first.newRecord);
    QVERIFY(first.newBestStreak);
    QVERIFY(first.newBestBvPerSecond);
    QCOMPARE(first.currentStreak, 1u);

    // Slower clock + slower rate — neither bestSeconds nor bestBvPerSecond
    // bumps; streak still extends.
    const auto slower = Stats::recordWin(QStringLiteral("Beginner"), 60.0, QDate{2026, 4, 24}, 0.50);
    QVERIFY(!slower.newRecord);
    QVERIFY(slower.newBestStreak); // streak crosses 2 > 1
    QVERIFY(!slower.newBestBvPerSecond);
    QCOMPARE(slower.currentStreak, 2u);
}

void TestStats::testBestFlagAccuracyDefaultsZero()
{
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.bestFlagAccuracyPercent, 0u);
    QVERIFY(!r.bestFlagAccuracyDate.isValid());
}

void TestStats::testRecordLossDefaultArgsKeepsBestFlagAccuracyZero()
{
    // Default-arg recordLoss (no flag-accuracy supplied) must not touch the
    // flag-accuracy best — preserves source-compat for callers that don't
    // care about the new field. Mirrors the bestSafePercent default-args
    // contract from v1.28.
    Stats::recordLoss(QStringLiteral("Beginner"));
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.played, 1u);
    QCOMPARE(r.bestFlagAccuracyPercent, 0u);
    QVERIFY(!r.bestFlagAccuracyDate.isValid());
}

void TestStats::testRecordLossWithFlagAccuracySetsOnFirstCall()
{
    const QDate d{2026, 4, 25};
    Stats::recordLoss(QStringLiteral("Beginner"), 0, 75, d);
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.played, 1u);
    QCOMPARE(r.bestFlagAccuracyPercent, 75u);
    QCOMPARE(r.bestFlagAccuracyDate, d);
}

void TestStats::testRecordLossWithHigherFlagAccuracyBeats()
{
    const QDate d1{2026, 1, 1};
    const QDate d2{2026, 4, 25};
    Stats::recordLoss(QStringLiteral("Expert"), 0, 40, d1);
    Stats::recordLoss(QStringLiteral("Expert"), 0, 80, d2);
    const auto r = Stats::load(QStringLiteral("Expert"));
    QCOMPARE(r.bestFlagAccuracyPercent, 80u);
    QCOMPARE(r.bestFlagAccuracyDate, d2);
}

void TestStats::testRecordLossWithLowerFlagAccuracyKeepsOriginal()
{
    const QDate d1{2026, 1, 1};
    const QDate d2{2026, 4, 25};
    Stats::recordLoss(QStringLiteral("Expert"), 0, 80, d1);
    Stats::recordLoss(QStringLiteral("Expert"), 0, 60, d2);
    const auto r = Stats::load(QStringLiteral("Expert"));
    QCOMPARE(r.bestFlagAccuracyPercent, 80u);
    QCOMPARE(r.bestFlagAccuracyDate, d1);
}

void TestStats::testRecordLossWithEqualFlagAccuracyKeepsOriginalDate()
{
    // Strict greater-than semantics: ties don't bump the date — mirrors the
    // best-streak / best-percent / best-3BV/s date-pin convention.
    const QDate d1{2026, 1, 1};
    const QDate d2{2026, 4, 25};
    Stats::recordLoss(QStringLiteral("Expert"), 0, 70, d1);
    Stats::recordLoss(QStringLiteral("Expert"), 0, 70, d2);
    const auto r = Stats::load(QStringLiteral("Expert"));
    QCOMPARE(r.bestFlagAccuracyPercent, 70u);
    QCOMPARE(r.bestFlagAccuracyDate, d1);
}

void TestStats::testRecordLossWithZeroFlagAccuracyDoesNotMutate()
{
    // First seed a non-zero best, then a zero-accuracy loss must leave it
    // (e.g. a no-flag boom or a wrong-only-flag loss). Mirrors the
    // bestSafePercent zero-sentinel.
    Stats::recordLoss(QStringLiteral("Expert"), 0, 60, QDate{2026, 1, 1});
    Stats::recordLoss(QStringLiteral("Expert"), 0, 0, QDate{2026, 4, 25});
    const auto r = Stats::load(QStringLiteral("Expert"));
    QCOMPARE(r.played, 2u);
    QCOMPARE(r.bestFlagAccuracyPercent, 60u);
    QCOMPARE(r.bestFlagAccuracyDate, (QDate{2026, 1, 1}));
}

void TestStats::testRecordLossWithFullFlagAccuracyBoundary()
{
    // 100% is a defensible boundary: a player who flagged only mines (every
    // flag correct) before stepping on an unflagged mine. The recording
    // layer accepts 100 without clamping.
    Stats::recordLoss(QStringLiteral("Expert"), 0, 100, QDate{2026, 4, 25});
    const auto r = Stats::load(QStringLiteral("Expert"));
    QCOMPARE(r.bestFlagAccuracyPercent, 100u);
}

void TestStats::testRecordLossWithOverflowFlagAccuracyClampedTo100()
{
    // Defence-in-depth: a future caller passing an overflowed value (e.g.
    // 200 from an arithmetic bug) must still produce a sane record. Mirrors
    // the bestSafePercent clamp test.
    Stats::recordLoss(QStringLiteral("Expert"), 0, 200, QDate{2026, 4, 25});
    const auto r = Stats::load(QStringLiteral("Expert"));
    QCOMPARE(r.bestFlagAccuracyPercent, 100u);
}

void TestStats::testRecordWinDoesNotTouchBestFlagAccuracy()
{
    // Seed a flag-accuracy best, then win — recordWin must not zero or
    // overwrite the field. The post-win flagAllMines auto-flags every mine
    // so any flag-accuracy reading on a win is trivially 100% and would
    // pollute the meaningful per-loss leaderboard.
    Stats::recordLoss(QStringLiteral("Expert"), 0, 75, QDate{2026, 1, 1});
    QVERIFY(Stats::recordWin(QStringLiteral("Expert"), 250.0, QDate{2026, 4, 25}));
    const auto r = Stats::load(QStringLiteral("Expert"));
    QCOMPARE(r.bestFlagAccuracyPercent, 75u);
    QCOMPARE(r.bestFlagAccuracyDate, (QDate{2026, 1, 1}));
    QCOMPARE(r.won, 1u);
}

void TestStats::testBestFlagAccuracyIsPerDifficulty()
{
    Stats::recordLoss(QStringLiteral("Beginner"), 0, 90, QDate{2026, 4, 25});
    Stats::recordLoss(QStringLiteral("Expert"), 0, 50, QDate{2026, 4, 25});
    QCOMPARE(Stats::load(QStringLiteral("Beginner")).bestFlagAccuracyPercent, 90u);
    QCOMPARE(Stats::load(QStringLiteral("Intermediate")).bestFlagAccuracyPercent, 0u);
    QCOMPARE(Stats::load(QStringLiteral("Expert")).bestFlagAccuracyPercent, 50u);
}

void TestStats::testResetWipesBestFlagAccuracy()
{
    Stats::recordLoss(QStringLiteral("Expert"), 0, 80, QDate{2026, 4, 25});
    Stats::reset(QStringLiteral("Expert"));
    const auto r = Stats::load(QStringLiteral("Expert"));
    QCOMPARE(r.bestFlagAccuracyPercent, 0u);
    QVERIFY(!r.bestFlagAccuracyDate.isValid());
}

void TestStats::testResetAllWipesBestFlagAccuracy()
{
    Stats::recordLoss(QStringLiteral("Beginner"), 0, 90, QDate{2026, 4, 25});
    Stats::recordLoss(QStringLiteral("Expert"), 0, 70, QDate{2026, 4, 25});
    Stats::resetAll();
    QCOMPARE(Stats::load(QStringLiteral("Beginner")).bestFlagAccuracyPercent, 0u);
    QCOMPARE(Stats::load(QStringLiteral("Expert")).bestFlagAccuracyPercent, 0u);
}

void TestStats::testLegacyRecordWithoutBestFlagAccuracyLoadsAsZero()
{
    // Pre-1.33 record: no best_flag_accuracy_* keys present — must load as
    // 0 / invalid date so an upgrading user's first 1.33.0 loss seeds the
    // record cleanly.
    QSettings settings;
    settings.setValue(QStringLiteral("stats/Beginner/played"), 5u);
    settings.setValue(QStringLiteral("stats/Beginner/won"), 3u);
    settings.setValue(QStringLiteral("stats/Beginner/best_seconds"), 42.0);
    settings.setValue(QStringLiteral("stats/Beginner/best_date"), QStringLiteral("2026-01-01"));
    settings.sync();

    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.played, 5u);
    QCOMPARE(r.bestSeconds, 42.0);
    QCOMPARE(r.bestFlagAccuracyPercent, 0u);
    QVERIFY(!r.bestFlagAccuracyDate.isValid());
}

void TestStats::testLossOutcomeFlagAccuracyIndependentOfSafePercent()
{
    // Independence pin: safePercent and flagAccuracyPercent are not coupled.
    // A loss with a higher safePercent and lower flag-accuracy must update
    // only the safePercent record, and vice versa.
    const QDate d1{2026, 4, 23};
    const QDate d2{2026, 4, 24};
    const QDate d3{2026, 4, 25};

    // First loss: 30% cleared, 60% flag-accuracy. Both axes set.
    {
        const auto o = Stats::recordLoss(QStringLiteral("Beginner"), 30, 60, d1);
        QVERIFY(o.newBestSafePercent);
        QVERIFY(o.newBestFlagAccuracyPercent);
    }
    // Second loss: 70% cleared (new safePercent record) but 40% accuracy
    // (worse). safePercent updates; flag-accuracy does not.
    {
        const auto o = Stats::recordLoss(QStringLiteral("Beginner"), 70, 40, d2);
        QVERIFY(o.newBestSafePercent);
        QVERIFY(!o.newBestFlagAccuracyPercent);
    }
    // Third loss: 50% cleared (worse than 70) but 80% accuracy (new
    // record). Flag-accuracy updates; safePercent does not.
    {
        const auto o = Stats::recordLoss(QStringLiteral("Beginner"), 50, 80, d3);
        QVERIFY(!o.newBestSafePercent);
        QVERIFY(o.newBestFlagAccuracyPercent);
    }
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.bestSafePercent, 70u);
    QCOMPARE(r.bestSafePercentDate, d2);
    QCOMPARE(r.bestFlagAccuracyPercent, 80u);
    QCOMPARE(r.bestFlagAccuracyDate, d3);
}

void TestStats::testLossOutcomeBoolStillTracksOnlySafePercent()
{
    // The explicit `operator bool` was added in v1.29 to gate the `🎯 New
    // best %!` flair. The 1.33.0 addition of `newBestFlagAccuracyPercent`
    // must NOT change that contract — bool conversion remains tied to
    // newBestSafePercent only, so the existing flair-gate keeps its
    // pre-1.33 semantics. A loss that sets a new flag-accuracy record but
    // not a new safe-percent record returns false.
    const auto out = Stats::recordLoss(QStringLiteral("Beginner"), 0, 70, QDate{2026, 4, 25});
    QVERIFY(!out.newBestSafePercent);
    QVERIFY(out.newBestFlagAccuracyPercent);
    QVERIFY(!static_cast<bool>(out));
}

// total_seconds_won: lifetime sum of winning durations. Drives the
// win-dialog "Average: %1" line via WinOutcome::averageSecondsAfter
// (gated at the call site on winsAfter >= 3).

void TestStats::testTotalSecondsWonDefaultsZero()
{
    // Empty record: no plist entry at all → load returns 0.0.
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.totalSecondsWon, 0.0);
}

void TestStats::testRecordWinAccumulatesTotalSeconds()
{
    Stats::recordWin(QStringLiteral("Beginner"), 15.5);
    QCOMPARE(Stats::load(QStringLiteral("Beginner")).totalSecondsWon, 15.5);
}

void TestStats::testMultipleWinsAccumulateTotalSeconds()
{
    // Three wins at 10/20/30 seconds → accumulator 60.0; mean 20.0 — the
    // canonical "ao3" speedrun average.
    Stats::recordWin(QStringLiteral("Beginner"), 10.0);
    Stats::recordWin(QStringLiteral("Beginner"), 20.0);
    Stats::recordWin(QStringLiteral("Beginner"), 30.0);
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.won, 3u);
    QCOMPARE(r.totalSecondsWon, 60.0);
}

void TestStats::testRecordLossDoesNotTouchTotalSecondsWon()
{
    // Pin: a loss only mutates played / streak / partial-best fields. The
    // win-time accumulator is owned by the win path exclusively.
    Stats::recordWin(QStringLiteral("Beginner"), 25.0);
    Stats::recordLoss(QStringLiteral("Beginner"));
    Stats::recordLoss(QStringLiteral("Beginner"));
    QCOMPARE(Stats::load(QStringLiteral("Beginner")).totalSecondsWon, 25.0);
}

void TestStats::testRecordWinZeroSecondsDoesNotAccumulate()
{
    // Sub-tick win sentinel (only reachable from setFixedLayout-driven
    // tests in production): seconds == 0.0 must not poison the divisor.
    // Mirrors the bestSeconds gate in the same recordWin path.
    Stats::recordWin(QStringLiteral("Beginner"), 0.0);
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.won, 1u); // win still counts toward the played/won totals
    QCOMPARE(r.totalSecondsWon, 0.0);
}

void TestStats::testWinOutcomeWinsAfterIsPostIncrement()
{
    // winsAfter mirrors r.won after the increment — the caller uses it as
    // a threshold gate (>= 3) without re-loading the record.
    QCOMPARE(Stats::recordWin(QStringLiteral("Beginner"), 10.0).winsAfter, 1u);
    QCOMPARE(Stats::recordWin(QStringLiteral("Beginner"), 12.0).winsAfter, 2u);
    QCOMPARE(Stats::recordWin(QStringLiteral("Beginner"), 14.0).winsAfter, 3u);
}

void TestStats::testWinOutcomeAverageSecondsAfterEqualsMean()
{
    // After three wins at 10/20/30, the third recordWin call must return
    // 20.0 as averageSecondsAfter — the value the win dialog renders.
    Stats::recordWin(QStringLiteral("Beginner"), 10.0);
    Stats::recordWin(QStringLiteral("Beginner"), 20.0);
    const auto out = Stats::recordWin(QStringLiteral("Beginner"), 30.0);
    QCOMPARE(out.winsAfter, 3u);
    QCOMPARE(out.averageSecondsAfter, 20.0);
}

void TestStats::testWinOutcomeAverageZeroOnSubTickFirstWin()
{
    // First win is sub-tick (seconds == 0.0): winsAfter == 1 but the
    // accumulator stays at 0.0 → averageSecondsAfter must be 0.0, not
    // a 0/1 division (which would also be 0.0 but for the wrong reason).
    // The 0.0 sentinel doubles as the "do not show Average line"
    // signal at the call site — neutralising the n=1 case below the
    // threshold even when the threshold check is bypassed.
    const auto out = Stats::recordWin(QStringLiteral("Beginner"), 0.0);
    QCOMPARE(out.winsAfter, 1u);
    QCOMPARE(out.averageSecondsAfter, 0.0);
}

void TestStats::testTotalSecondsWonIsPerDifficulty()
{
    // Independence pin: an Expert win must not touch Beginner's
    // accumulator and vice versa. Same shape as the bestSeconds /
    // bestBvPerSecond / bestFlagAccuracy independence tests.
    Stats::recordWin(QStringLiteral("Beginner"), 10.0);
    Stats::recordWin(QStringLiteral("Beginner"), 20.0);
    Stats::recordWin(QStringLiteral("Expert"), 200.0);
    QCOMPARE(Stats::load(QStringLiteral("Beginner")).totalSecondsWon, 30.0);
    QCOMPARE(Stats::load(QStringLiteral("Intermediate")).totalSecondsWon, 0.0);
    QCOMPARE(Stats::load(QStringLiteral("Expert")).totalSecondsWon, 200.0);
}

void TestStats::testResetWipesTotalSecondsWon()
{
    Stats::recordWin(QStringLiteral("Expert"), 250.0);
    Stats::reset(QStringLiteral("Expert"));
    QCOMPARE(Stats::load(QStringLiteral("Expert")).totalSecondsWon, 0.0);
}

void TestStats::testResetAllWipesTotalSecondsWon()
{
    Stats::recordWin(QStringLiteral("Beginner"), 10.0);
    Stats::recordWin(QStringLiteral("Expert"), 200.0);
    Stats::resetAll();
    QCOMPARE(Stats::load(QStringLiteral("Beginner")).totalSecondsWon, 0.0);
    QCOMPARE(Stats::load(QStringLiteral("Expert")).totalSecondsWon, 0.0);
}

void TestStats::testLegacyRecordWithoutTotalSecondsWonLoadsAsZero()
{
    // Pre-1.36 record: no `total_seconds_won` key present — must load as
    // 0.0 so an upgrading user's first 1.36+ win seeds the accumulator
    // cleanly, rather than carrying a `bestSeconds × won` overestimate
    // that would make the first displayed averages optimistically wrong.
    QSettings settings;
    settings.setValue(QStringLiteral("stats/Beginner/played"), 5u);
    settings.setValue(QStringLiteral("stats/Beginner/won"), 4u);
    settings.setValue(QStringLiteral("stats/Beginner/best_seconds"), 12.5);
    settings.setValue(QStringLiteral("stats/Beginner/best_date"), QStringLiteral("2026-01-01"));
    settings.sync();

    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.played, 5u);
    QCOMPARE(r.won, 4u);
    QCOMPARE(r.bestSeconds, 12.5);
    QCOMPARE(r.totalSecondsWon, 0.0);
}

// WinOutcome.bestSecondsAfter — populated post-update so the win dialog can
// render the `Average: %1 (best %2)` companion suffix without re-loading.

void TestStats::testWinOutcomeDefaultBestSecondsAfterIsZero()
{
    // A default-constructed WinOutcome (the value MainWindow uses for replays
    // / custom games that never call recordWin) carries 0.0. The win-dialog
    // call site uses this 0.0 as the "do not render the (best %1) suffix"
    // sentinel — the same sentinel `bestSeconds` itself uses for an empty
    // record.
    Stats::WinOutcome out{};
    QCOMPARE(out.bestSecondsAfter, 0.0);
}

void TestStats::testWinOutcomeBestSecondsAfterFirstWinEqualsSeconds()
{
    // First counted non-sub-tick win sets bestSeconds to the win duration;
    // bestSecondsAfter must mirror that value (it's the same field, read
    // post-mutation). winsAfter == 1 here so the win dialog wouldn't yet
    // render Average; this test pins the WinOutcome wiring regardless of
    // the dialog's >= 3 gate.
    const auto out = Stats::recordWin(QStringLiteral("Beginner"), 30.0);
    QCOMPARE(out.bestSecondsAfter, 30.0);
}

void TestStats::testWinOutcomeBestSecondsAfterTracksFasterWin()
{
    // Three wins at 30 → 20 → 25. Best transitions 30 → 20 on win 2, holds
    // 20 on win 3 (slower). bestSecondsAfter on the final call is 20.0,
    // distinct from averageSecondsAfter (25.0) — the very point of the
    // companion suffix is surfacing the gap.
    Stats::recordWin(QStringLiteral("Beginner"), 30.0);
    Stats::recordWin(QStringLiteral("Beginner"), 20.0);
    const auto out = Stats::recordWin(QStringLiteral("Beginner"), 25.0);
    QCOMPARE(out.winsAfter, 3u);
    QCOMPARE(out.averageSecondsAfter, 25.0);
    QCOMPARE(out.bestSecondsAfter, 20.0);
}

void TestStats::testWinOutcomeBestSecondsAfterKeepsPriorOnSlowerWin()
{
    // Pin the slower-win-doesn't-touch-best path through the WinOutcome.
    // After 20 then 40, bestSecondsAfter == 20.0 (not 40.0) on the second
    // call. Mirror of testSlowerWinKeepsBest but reads the field that
    // drives the win-dialog suffix instead of the persisted record.
    Stats::recordWin(QStringLiteral("Beginner"), 20.0);
    const auto out = Stats::recordWin(QStringLiteral("Beginner"), 40.0);
    QCOMPARE(out.bestSecondsAfter, 20.0);
}

void TestStats::testWinOutcomeBestSecondsAfterSubTickWinKeepsPrior()
{
    // Sub-tick win after a real win: bestSeconds doesn't update (gated on
    // seconds > 0.0 in recordWin), so bestSecondsAfter == prior best.
    Stats::recordWin(QStringLiteral("Beginner"), 30.0);
    const auto out = Stats::recordWin(QStringLiteral("Beginner"), 0.0);
    QCOMPARE(out.bestSecondsAfter, 30.0);
}

void TestStats::testWinOutcomeBestSecondsAfterFirstWinSubTickStaysZero()
{
    // First-ever win is sub-tick: bestSeconds stays 0.0, so
    // bestSecondsAfter == 0.0. Important pin for the win-dialog gate —
    // when (theoretically) winsAfter >= 3 with all-sub-tick wins,
    // averageSecondsAfter is 0.0 and so is bestSecondsAfter, and the
    // call site's `winAverageSeconds > 0.0` gate hides BOTH lines.
    const auto out = Stats::recordWin(QStringLiteral("Beginner"), 0.0);
    QCOMPARE(out.bestSecondsAfter, 0.0);
}

void TestStats::testWinOutcomeBestSecondsAfterMatchesPersisted()
{
    // Cross-pin: bestSecondsAfter is the same value the next Stats::load
    // would return. Guards against a refactor that diverges the in-memory
    // pre-save value from what the call site sees on the next dialog open.
    Stats::recordWin(QStringLiteral("Expert"), 250.0);
    const auto out = Stats::recordWin(QStringLiteral("Expert"), 200.0);
    const auto r = Stats::load(QStringLiteral("Expert"));
    QCOMPARE(out.bestSecondsAfter, r.bestSeconds);
    QCOMPARE(out.bestSecondsAfter, 200.0);
}

void TestStats::testLastWinDateDefaultsInvalid()
{
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QVERIFY(!r.lastWinDate.isValid());
}

void TestStats::testFirstWinStampsLastWinDate()
{
    const QDate d{2026, 4, 23};
    QVERIFY(Stats::recordWin(QStringLiteral("Beginner"), 30.0, d));
    QCOMPARE(Stats::load(QStringLiteral("Beginner")).lastWinDate, d);
}

void TestStats::testSlowerWinOverwritesLastWinDateEvenWhenBestUnchanged()
{
    // The diverging behaviour from `bestDate`: a slower follow-up win
    // does NOT touch `bestDate` (the original best-time run keeps it),
    // but DOES overwrite `lastWinDate` (this is the most-recent win,
    // not the fastest). Mirrors the "every counted win" semantic.
    const QDate originalDate{2026, 1, 1};
    const QDate laterDate{2026, 4, 23};
    QVERIFY(Stats::recordWin(QStringLiteral("Beginner"), 20.0, originalDate));
    QVERIFY(!Stats::recordWin(QStringLiteral("Beginner"), 40.0, laterDate));
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.bestDate, originalDate); // best stayed
    QCOMPARE(r.lastWinDate, laterDate); // last advanced
}

void TestStats::testFasterWinAlsoOverwritesLastWinDate()
{
    const QDate originalDate{2026, 1, 1};
    const QDate fasterDate{2026, 4, 23};
    QVERIFY(Stats::recordWin(QStringLiteral("Beginner"), 30.0, originalDate));
    QVERIFY(Stats::recordWin(QStringLiteral("Beginner"), 20.0, fasterDate));
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.bestDate, fasterDate);    // best advanced
    QCOMPARE(r.lastWinDate, fasterDate); // and so did last
}

void TestStats::testLossDoesNotTouchLastWinDate()
{
    const QDate winDate{2026, 1, 1};
    QVERIFY(Stats::recordWin(QStringLiteral("Beginner"), 20.0, winDate));
    Stats::recordLoss(QStringLiteral("Beginner"));
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.lastWinDate, winDate);
    QCOMPARE(r.played, 2u);
    QCOMPARE(r.won, 1u);
}

void TestStats::testSubTickWinStampsLastWinDate()
{
    // A 0.0-seconds win still counts as a win (won/played both increment),
    // so it must stamp lastWinDate too — the date is meaningful regardless
    // of duration, unlike the bestSeconds / totalSecondsWon updates which
    // gate on `seconds > 0.0`.
    const QDate d{2026, 4, 23};
    Stats::recordWin(QStringLiteral("Beginner"), 0.0, d);
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.won, 1u);
    QCOMPARE(r.bestSeconds, 0.0); // best stays at sentinel
    QCOMPARE(r.lastWinDate, d);   // but last-win date is stamped
}

void TestStats::testLastWinDateIsPerDifficulty()
{
    const QDate beginnerDate{2026, 4, 1};
    const QDate expertDate{2026, 4, 23};
    Stats::recordWin(QStringLiteral("Beginner"), 10.0, beginnerDate);
    Stats::recordWin(QStringLiteral("Expert"), 200.0, expertDate);
    QCOMPARE(Stats::load(QStringLiteral("Beginner")).lastWinDate, beginnerDate);
    QVERIFY(!Stats::load(QStringLiteral("Intermediate")).lastWinDate.isValid());
    QCOMPARE(Stats::load(QStringLiteral("Expert")).lastWinDate, expertDate);
}

void TestStats::testResetWipesLastWinDate()
{
    QVERIFY(Stats::recordWin(QStringLiteral("Beginner"), 20.0, QDate{2026, 1, 1}));
    Stats::reset(QStringLiteral("Beginner"));
    QVERIFY(!Stats::load(QStringLiteral("Beginner")).lastWinDate.isValid());
}

void TestStats::testResetAllWipesLastWinDate()
{
    Stats::recordWin(QStringLiteral("Beginner"), 10.0, QDate{2026, 4, 1});
    Stats::recordWin(QStringLiteral("Expert"), 200.0, QDate{2026, 4, 23});
    Stats::resetAll();
    QVERIFY(!Stats::load(QStringLiteral("Beginner")).lastWinDate.isValid());
    QVERIFY(!Stats::load(QStringLiteral("Expert")).lastWinDate.isValid());
}

void TestStats::testLegacyRecordWithoutLastWinDateLoadsAsInvalid()
{
    // Pre-1.37 record: no `last_win_date` key present. Must load as
    // an invalid QDate so the loss-dialog gate (`isValid()`) keeps the
    // line hidden until the player's next 1.37+ win.
    QSettings settings;
    settings.setValue(QStringLiteral("stats/Beginner/played"), 5u);
    settings.setValue(QStringLiteral("stats/Beginner/won"), 3u);
    settings.setValue(QStringLiteral("stats/Beginner/best_seconds"), 12.5);
    settings.setValue(QStringLiteral("stats/Beginner/best_date"), QStringLiteral("2026-01-01"));
    settings.sync();

    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.won, 3u);
    QVERIFY(!r.lastWinDate.isValid());
}

void TestStats::testLegacyRecordWithWonButNoLastWinDateLoadsAsInvalid()
{
    // Defensive: even with `won > 0`, an absent `last_win_date` must NOT
    // be back-filled from `best_date`. The loss dialog's gate is purely
    // `isValid()` — we want a clean miss until the next counted win
    // refreshes it, rather than silently aliasing `bestDate` (which may
    // be months stale even if the user won yesterday).
    QSettings settings;
    settings.setValue(QStringLiteral("stats/Expert/played"), 100u);
    settings.setValue(QStringLiteral("stats/Expert/won"), 50u);
    settings.setValue(QStringLiteral("stats/Expert/best_seconds"), 250.0);
    settings.setValue(QStringLiteral("stats/Expert/best_date"), QStringLiteral("2025-06-01"));
    settings.sync();

    const auto r = Stats::load(QStringLiteral("Expert"));
    QCOMPARE(r.won, 50u);
    QCOMPARE(r.bestDate, QDate(2025, 6, 1));
    QVERIFY(!r.lastWinDate.isValid()); // not back-filled
}

// Loss-dialog "Average: %1 (best %2)" line — these tests pin the
// loaded-record contract `MainWindow::onGameLost` reads. The win path
// threads the mean through WinOutcome.{averageSecondsAfter,bestSecondsAfter}
// (covered by the WinOutcome tests above); the loss path has no
// recordLoss-returned field for the mean, so it computes from
// `Stats::load(diff).{won,totalSecondsWon,bestSeconds}` directly with the
// same `won >= 3 && totalSecondsWon > 0.0` gate the win-side encodes via
// `winsAfter >= 3`. These tests pin that loss-side contract explicitly so a
// future refactor of the load shape can't silently break the loss-dialog
// render gate.

void TestStats::testLoadAfterThreeWinsExposesAverageForLossDialog()
{
    // Canonical n=3 ao3 mean (10/20/30 → 20.0). Pinned against the same
    // (load → divide) sequence the loss path runs every game-lost dispatch.
    Stats::recordWin(QStringLiteral("Beginner"), 10.0);
    Stats::recordWin(QStringLiteral("Beginner"), 20.0);
    Stats::recordWin(QStringLiteral("Beginner"), 30.0);
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.won, 3u);
    QCOMPARE(r.totalSecondsWon, 60.0);
    QVERIFY(r.totalSecondsWon > 0.0); // gate satisfied
    QCOMPARE(r.totalSecondsWon / r.won, 20.0);
}

void TestStats::testLoadAfterThreeWinsExposesBestForLossDialog()
{
    // Companion: when the loss-side Average line renders, the (best %1)
    // suffix anchors against r.bestSeconds (= fastest of the three wins).
    Stats::recordWin(QStringLiteral("Beginner"), 30.0);
    Stats::recordWin(QStringLiteral("Beginner"), 18.0); // new best
    Stats::recordWin(QStringLiteral("Beginner"), 24.0);
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.won, 3u);
    QCOMPARE(r.bestSeconds, 18.0);
    QCOMPARE(r.totalSecondsWon, 72.0);
    QVERIFY(r.bestSeconds > 0.0); // (best %1) suffix gate satisfied
}

void TestStats::testLoadAfterTwoWinsBelowLossDialogGate()
{
    // Below the gate (n=2): the persisted record is internally consistent
    // (won=2, totalSecondsWon=mean·n), but the loss-side `won >= 3` gate
    // hides the line. Same threshold reasoning as the win-side: n<3 is
    // not informative ("the average is the best" / "single data point").
    Stats::recordWin(QStringLiteral("Beginner"), 10.0);
    Stats::recordWin(QStringLiteral("Beginner"), 20.0);
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.won, 2u);
    QCOMPARE(r.totalSecondsWon, 30.0);
    QVERIFY(r.won < 3u); // gate-closed: loss dialog must hide
}

void TestStats::testLoadAfterAllSubTickWinsLeavesLossDialogGateClosed()
{
    // Pathological all-sub-tick case: won=3 (gate's n threshold satisfied)
    // but totalSecondsWon stays 0.0 because every recordWin gated on
    // `seconds > 0.0`. The loss-side compound gate (`won>=3 && total>0.0`)
    // still hides the line — same defensive divisor guard the win-side
    // applies in `recordWin` itself.
    Stats::recordWin(QStringLiteral("Beginner"), 0.0);
    Stats::recordWin(QStringLiteral("Beginner"), 0.0);
    Stats::recordWin(QStringLiteral("Beginner"), 0.0);
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.won, 3u);
    QCOMPARE(r.totalSecondsWon, 0.0);
    QCOMPARE(r.bestSeconds, 0.0); // suffix would be hidden too
}

void TestStats::testLoadAfterMixedSubTickAndRealWinsForLossDialog()
{
    // Mixed sub-tick + real wins: the divisor stays at the *count of all
    // counted wins* (won=3 here), but the numerator only includes the
    // real ones (12.0 + 0.0 + 18.0 = 30.0). The mean (10.0) is therefore
    // ~lower than it would be if we'd divided by the real-only count (2),
    // matching the win-side's `recordWin` behaviour. This pins the
    // loss-side contract against a refactor that switches the divisor.
    Stats::recordWin(QStringLiteral("Beginner"), 12.0);
    Stats::recordWin(QStringLiteral("Beginner"), 0.0); // sub-tick counts toward won
    Stats::recordWin(QStringLiteral("Beginner"), 18.0);
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.won, 3u);
    QCOMPARE(r.totalSecondsWon, 30.0);
    QCOMPARE(r.totalSecondsWon / r.won, 10.0);
    QCOMPARE(r.bestSeconds, 12.0); // best ignores sub-tick same as accumulator
}

void TestStats::testLoadAfterLossDoesNotDisturbAverageForLossDialog()
{
    // Pin: a loss after 3 wins must leave the loss-side render numbers
    // identical (recordLoss touches played/streak/partial-best fields, NOT
    // totalSecondsWon / won / bestSeconds). The loss dialog opens *after*
    // recordLoss runs in MainWindow::onGameLost, so this is the precise
    // state the dialog sees. (Aside from priorRecord being read pre-recordLoss,
    // which doesn't matter for the win-history fields.)
    Stats::recordWin(QStringLiteral("Beginner"), 10.0);
    Stats::recordWin(QStringLiteral("Beginner"), 20.0);
    Stats::recordWin(QStringLiteral("Beginner"), 30.0);
    Stats::recordLoss(QStringLiteral("Beginner"));
    const auto r = Stats::load(QStringLiteral("Beginner"));
    QCOMPARE(r.played, 4u);
    QCOMPARE(r.won, 3u); // unchanged
    QCOMPARE(r.totalSecondsWon, 60.0);
    QCOMPARE(r.bestSeconds, 10.0);
    QCOMPARE(r.totalSecondsWon / r.won, 20.0); // average unchanged
}

// Win-dialog "✨ Beat your average!" flair gate — the gate compares the
// just-finished `seconds` against `WinOutcome::averageSecondsAfter` (which
// already includes the current win in its mean). These tests pin the
// arithmetic the gate consumes without instantiating MainWindow.

void TestStats::testBeatAverageGateFiresWhenSecondsBelowMean()
{
    // 30/30/30 then 20: the 4th call returns averageSecondsAfter = 27.5
    // (110/4). The gate compares the just-finished seconds (20) to that
    // post-update mean — 20 < 27.5 → flair fires. Pin both sides of the
    // comparison so the relationship is explicit.
    Stats::recordWin(QStringLiteral("Beginner"), 30.0);
    Stats::recordWin(QStringLiteral("Beginner"), 30.0);
    Stats::recordWin(QStringLiteral("Beginner"), 30.0);
    const double seconds = 20.0;
    const auto out = Stats::recordWin(QStringLiteral("Beginner"), seconds);
    QCOMPARE(out.winsAfter, 4u);
    QCOMPARE(out.averageSecondsAfter, 27.5);
    QVERIFY(seconds < out.averageSecondsAfter);
}

void TestStats::testBeatAverageGateClosedWhenSecondsAtMean()
{
    // Tying the prior mean leaves the post-update mean equal to seconds —
    // the strict `<` comparison hides the flair. "Beat" requires beating,
    // not matching.
    Stats::recordWin(QStringLiteral("Beginner"), 20.0);
    Stats::recordWin(QStringLiteral("Beginner"), 20.0);
    Stats::recordWin(QStringLiteral("Beginner"), 20.0);
    const double seconds = 20.0;
    const auto out = Stats::recordWin(QStringLiteral("Beginner"), seconds);
    QCOMPARE(out.winsAfter, 4u);
    QCOMPARE(out.averageSecondsAfter, 20.0);
    QVERIFY(!(seconds < out.averageSecondsAfter));
}

void TestStats::testBeatAverageGateClosedWhenSecondsAboveMean()
{
    // 20/20/20 then 30: the post-update mean = 90/4 = 22.5. seconds (30)
    // is above mean → flair hides. Pin both numbers because a buggy
    // implementation that compared seconds to the *prior* mean (20) would
    // also evaluate to false here, masking the bug — explicit values guard
    // against that.
    Stats::recordWin(QStringLiteral("Beginner"), 20.0);
    Stats::recordWin(QStringLiteral("Beginner"), 20.0);
    Stats::recordWin(QStringLiteral("Beginner"), 20.0);
    const double seconds = 30.0;
    const auto out = Stats::recordWin(QStringLiteral("Beginner"), seconds);
    QCOMPARE(out.winsAfter, 4u);
    QCOMPARE(out.averageSecondsAfter, 22.5);
    QVERIFY(!(seconds < out.averageSecondsAfter));
}

void TestStats::testBeatAverageGateRequiresThreeWinsCallerSide()
{
    // The MainWindow call site passes `winAverageSeconds = 0.0` when
    // `outcome.winsAfter < 3`, so the dialog gate (`> 0.0`) hides the
    // flair regardless of the post-update mean. Pin both sub-threshold
    // counts: first win and second win.
    const auto first = Stats::recordWin(QStringLiteral("Beginner"), 30.0);
    QCOMPARE(first.winsAfter, 1u);
    QVERIFY(first.winsAfter < 3u);
    const auto second = Stats::recordWin(QStringLiteral("Beginner"), 10.0);
    QCOMPARE(second.winsAfter, 2u);
    QVERIFY(second.winsAfter < 3u);
    // Third win crosses the threshold — flair becomes eligible (whether
    // it actually fires depends on the seconds-vs-mean comparison, which
    // the other tests pin).
    const auto third = Stats::recordWin(QStringLiteral("Beginner"), 20.0);
    QCOMPARE(third.winsAfter, 3u);
}

void TestStats::testBeatAverageGateNotPoisonedBySubTickWinsAfterRealWins()
{
    // Sub-tick wins (seconds == 0.0) increment `won` but do not add to
    // `totalSecondsWon` (gated in recordWin). After 30/30/30 then 0.0 the
    // post-update averageSecondsAfter = 90/4 = 22.5 — the divisor moved but
    // the numerator didn't. The dialog's `m_lastElapsedSeconds > 0.0`
    // defensive guard hides the flair anyway (a 0.0 "win" is not an
    // achievement to flag); pin the underlying arithmetic so a future
    // refactor of recordWin can't silently break the assumption.
    Stats::recordWin(QStringLiteral("Beginner"), 30.0);
    Stats::recordWin(QStringLiteral("Beginner"), 30.0);
    Stats::recordWin(QStringLiteral("Beginner"), 30.0);
    const auto out = Stats::recordWin(QStringLiteral("Beginner"), 0.0);
    QCOMPARE(out.winsAfter, 4u);
    QCOMPARE(out.averageSecondsAfter, 22.5);
}

void TestStats::testBeatAverageGateNotMutatedByLoss()
{
    // recordLoss between wins cannot shift the post-update mean — the
    // accumulator and `won` are won-path-exclusive. After 30/30/30 then
    // a loss then 20 → averageSecondsAfter is still 110/4 = 27.5. Same
    // guarantee as the loss-side `Average:` line, but pinned against the
    // win-side flair gate's specific data path.
    Stats::recordWin(QStringLiteral("Beginner"), 30.0);
    Stats::recordWin(QStringLiteral("Beginner"), 30.0);
    Stats::recordWin(QStringLiteral("Beginner"), 30.0);
    Stats::recordLoss(QStringLiteral("Beginner"));
    const double seconds = 20.0;
    const auto out = Stats::recordWin(QStringLiteral("Beginner"), seconds);
    QCOMPARE(out.winsAfter, 4u);
    QCOMPARE(out.averageSecondsAfter, 27.5);
    QVERIFY(seconds < out.averageSecondsAfter);
}

void TestStats::testLossOutcomeDefaultPriorStreakIsZero()
{
    // A default-constructed LossOutcome (e.g. the replay/custom branch in
    // MainWindow::onGameLost that skips recordLoss) must report priorStreak=0
    // so the loss dialog's `>= 2` gate cleanly hides the line.
    Stats::LossOutcome out{};
    QCOMPARE(out.priorStreak, 0u);
}

void TestStats::testFirstEverLossPriorStreakIsZero()
{
    // No prior wins → currentStreak was 0 at recordLoss time → priorStreak is 0.
    const auto out = Stats::recordLoss(QStringLiteral("Beginner"));
    QCOMPARE(out.priorStreak, 0u);
    // Sanity: the underlying record is still in the no-streak state.
    QCOMPARE(Stats::load(QStringLiteral("Beginner")).currentStreak, 0u);
}

void TestStats::testLossAfterOneWinPriorStreakIsOne()
{
    // A single win followed by a loss reports priorStreak=1. The dialog's
    // `>= 2` gate hides the line for this case (one isn't a streak), but the
    // underlying field carries the raw value so future callers / telemetry
    // can read it without re-loading the record.
    Stats::recordWin(QStringLiteral("Beginner"), 20.0);
    const auto out = Stats::recordLoss(QStringLiteral("Beginner"));
    QCOMPARE(out.priorStreak, 1u);
    QCOMPARE(Stats::load(QStringLiteral("Beginner")).currentStreak, 0u);
}

void TestStats::testLossAfterFiveWinsPriorStreakIsFive()
{
    // Five consecutive wins, then a loss. priorStreak captures the value at
    // recordLoss time, before the reset. Dialog gate `>= 2` is satisfied so
    // the line "💔 Streak ended at 5" surfaces.
    for (int i = 0; i < 5; ++i)
    {
        Stats::recordWin(QStringLiteral("Expert"), 100.0 - i);
    }
    QCOMPARE(Stats::load(QStringLiteral("Expert")).currentStreak, 5u);
    const auto out = Stats::recordLoss(QStringLiteral("Expert"));
    QCOMPARE(out.priorStreak, 5u);
    QCOMPARE(Stats::load(QStringLiteral("Expert")).currentStreak, 0u);
}

void TestStats::testTwoConsecutiveLossesSecondPriorStreakIsZero()
{
    // After the first loss, currentStreak is already 0. A second loss
    // immediately afterwards must report priorStreak=0 — there's no streak
    // left to break.
    Stats::recordWin(QStringLiteral("Beginner"), 10.0);
    Stats::recordWin(QStringLiteral("Beginner"), 9.0);
    const auto firstLoss = Stats::recordLoss(QStringLiteral("Beginner"));
    QCOMPARE(firstLoss.priorStreak, 2u);
    const auto secondLoss = Stats::recordLoss(QStringLiteral("Beginner"));
    QCOMPARE(secondLoss.priorStreak, 0u);
}

void TestStats::testWinThenLossThenWinThenLossPriorStreakResets()
{
    // win-loss-win-loss exercises the "current resets to 0 on loss, then
    // climbs again" path: each loss should report exactly the streak length
    // built up since the previous loss.
    Stats::recordWin(QStringLiteral("Beginner"), 10.0);
    Stats::recordWin(QStringLiteral("Beginner"), 11.0);
    Stats::recordWin(QStringLiteral("Beginner"), 12.0);
    const auto firstLoss = Stats::recordLoss(QStringLiteral("Beginner"));
    QCOMPARE(firstLoss.priorStreak, 3u);

    Stats::recordWin(QStringLiteral("Beginner"), 13.0);
    Stats::recordWin(QStringLiteral("Beginner"), 14.0);
    const auto secondLoss = Stats::recordLoss(QStringLiteral("Beginner"));
    QCOMPARE(secondLoss.priorStreak, 2u);
}

void TestStats::testPriorStreakIsPerDifficulty()
{
    // Win twice on Beginner, lose on Expert. Expert had no prior wins so
    // its priorStreak must be 0; Beginner's streak stays at 2 (untouched
    // by the Expert loss).
    Stats::recordWin(QStringLiteral("Beginner"), 10.0);
    Stats::recordWin(QStringLiteral("Beginner"), 11.0);
    const auto expertLoss = Stats::recordLoss(QStringLiteral("Expert"));
    QCOMPARE(expertLoss.priorStreak, 0u);
    QCOMPARE(Stats::load(QStringLiteral("Beginner")).currentStreak, 2u);
}

void TestStats::testRecordLossZerosCurrentStreakRegardlessOfPriorValue()
{
    // The post-loss `currentStreak` is always 0, no matter what `priorStreak`
    // reports. Defensive — guards against a future refactor that might leak
    // priorStreak into the persisted record.
    for (int i = 0; i < 7; ++i)
    {
        Stats::recordWin(QStringLiteral("Intermediate"), 50.0 - i);
    }
    Stats::recordLoss(QStringLiteral("Intermediate"));
    QCOMPARE(Stats::load(QStringLiteral("Intermediate")).currentStreak, 0u);
}

QTEST_MAIN(TestStats)
#include "tst_stats.moc"
