#include "../average_time_format.h"

#include <QtTest>

#include <cmath>
#include <cstdint>
#include <limits>

class TestAverageTimeFormat : public QObject
{
    Q_OBJECT

  private slots:
    void testZeroWonRendersAsEmDash();
    void testZeroTotalSecondsRendersAsEmDash();
    void testNegativeTotalSecondsRendersAsEmDash();
    void testSingleWinRendersThatWinsDuration();
    void testThreeWinsRendersMean();
    void testMeanCrossesMinuteBoundaryUsesClockFormat();
    void testMeanCrossesHourBoundaryUsesClockFormat();
    void testSubSecondMeanRendersOneDecimal();
    void testLargeWonCountDoesNotOverflow();
    void testFractionalMeanRoundsToTenths();
    void testZeroWonAndZeroSecondsBothEmDash();
    void testHugeWonWithTinyTotalEmDashIfTotalNonPositive();
};

void TestAverageTimeFormat::testZeroWonRendersAsEmDash()
{
    // Brand-new player — never won. Divide-by-zero guard fires before any
    // arithmetic. Mirrors the em-dash sentinel used by Best-time and
    // Last-win cells in the same dialog.
    QCOMPARE(formatAverageCell(0.0, 0u), QStringLiteral("—"));
    // Defensive: even a non-zero numerator with a zero denominator must
    // hit the em-dash branch (this can't happen in production — the
    // accumulator only grows on a recordWin call that also increments
    // won — but the guard order is total-seconds-then-won so this pin
    // catches a future refactor that swaps them).
    QCOMPARE(formatAverageCell(42.0, 0u), QStringLiteral("—"));
}

void TestAverageTimeFormat::testZeroTotalSecondsRendersAsEmDash()
{
    // Every counted win was sub-tick (seconds == 0.0). The accumulator
    // didn't grow, so the mean is mathematically 0.0 but not useful —
    // surface "—" rather than "0.0", matching the v1.36 win-dialog
    // Average-line gate.
    QCOMPARE(formatAverageCell(0.0, 1u), QStringLiteral("—"));
    QCOMPARE(formatAverageCell(0.0, 5u), QStringLiteral("—"));
}

void TestAverageTimeFormat::testNegativeTotalSecondsRendersAsEmDash()
{
    // Defensive: a negative accumulator is unreachable in production
    // (Stats::recordWin only adds non-negative values, and the QSettings
    // load path defaults to 0.0). Pin the em-dash fallback so a future
    // refactor that allows negatives can't render "-1:30.5" in the cell.
    QCOMPARE(formatAverageCell(-1.5, 1u), QStringLiteral("—"));
    QCOMPARE(formatAverageCell(-100.0, 10u), QStringLiteral("—"));
}

void TestAverageTimeFormat::testSingleWinRendersThatWinsDuration()
{
    // n=1 → mean is the single duration. Sub-60s -> "S.S".
    QCOMPARE(formatAverageCell(15.5, 1u), QStringLiteral("15.5"));
}

void TestAverageTimeFormat::testThreeWinsRendersMean()
{
    // Canonical three-win average: 10/20/30 -> mean 20.0 -> "20.0".
    // The same scenario the v1.36 win-dialog Average line tests against
    // (testWinOutcomeAverageSecondsAfterEqualsMean).
    QCOMPARE(formatAverageCell(60.0, 3u), QStringLiteral("20.0"));
}

void TestAverageTimeFormat::testMeanCrossesMinuteBoundaryUsesClockFormat()
{
    // Two Intermediate wins at 60s + 90s -> mean 75.0 -> "1:15.0".
    // formatElapsedTime owns the sub-60 vs M:SS.S vs H:MM:SS.S branching;
    // this test pins that the average path delegates correctly.
    QCOMPARE(formatAverageCell(150.0, 2u), QStringLiteral("1:15.0"));
}

void TestAverageTimeFormat::testMeanCrossesHourBoundaryUsesClockFormat()
{
    // Two Expert wins at 1h + 1h 30m -> mean 1h 15m 0s -> "1:15:00.0".
    // 3600 + 5400 = 9000s; mean 4500s = 1:15:00.0.
    QCOMPARE(formatAverageCell(9000.0, 2u), QStringLiteral("1:15:00.0"));
}

void TestAverageTimeFormat::testSubSecondMeanRendersOneDecimal()
{
    // Two sub-tick-ish wins at 0.5s each -> mean 0.5 -> "0.5".
    QCOMPARE(formatAverageCell(1.0, 2u), QStringLiteral("0.5"));
}

void TestAverageTimeFormat::testLargeWonCountDoesNotOverflow()
{
    // 10000 wins at 12.345s each -> mean 12.345 -> rounds to "12.3".
    QCOMPARE(formatAverageCell(123450.0, 10000u), QStringLiteral("12.3"));
}

void TestAverageTimeFormat::testFractionalMeanRoundsToTenths()
{
    // Three wins summing to 45.7s -> mean 15.2333... -> rounds to "15.2".
    QCOMPARE(formatAverageCell(45.7, 3u), QStringLiteral("15.2"));
    // And one that rounds up: 45.8 / 3 = 15.2666... -> "15.3".
    QCOMPARE(formatAverageCell(45.8, 3u), QStringLiteral("15.3"));
}

void TestAverageTimeFormat::testZeroWonAndZeroSecondsBothEmDash()
{
    // Tightest "fresh slate" pin — both fields default-constructed.
    QCOMPARE(formatAverageCell(0.0, 0u), QStringLiteral("—"));
}

void TestAverageTimeFormat::testHugeWonWithTinyTotalEmDashIfTotalNonPositive()
{
    // Defensive: the gate triggers on totalSecondsWon <= 0.0 even with
    // a huge denominator. Production can't reach this state (recordWin
    // only ever grows the accumulator), but the pin documents the gate
    // ordering.
    QCOMPARE(formatAverageCell(0.0, 999999u), QStringLiteral("—"));
}

QTEST_MAIN(TestAverageTimeFormat)
#include "tst_average_time_format.moc"
