#include "../time_format.h"

#include <QtTest>

#include <cmath>
#include <limits>

class TestTimeFormat : public QObject
{
    Q_OBJECT

  private slots:
    void testZero();
    void testSubMinute();
    void testJustUnderMinute();
    void testExactlyMinute();
    void testJustOverMinute();
    void testTwoDigitMinutes();
    void testJustUnderHour();
    void testExactlyHour();
    void testMultiHour();
    void testTenthsRoundUpAcrossSecondBoundary();
    void testTenthsRoundUpAcrossMinuteBoundary();
    void testNegativeRendersAsZero();
    void testNonFiniteRendersAsZero();
    void testFractionFormattingPreservesTrailingZero();
};

void TestTimeFormat::testZero() { QCOMPARE(formatElapsedTime(0.0), QStringLiteral("0.0")); }

void TestTimeFormat::testSubMinute()
{
    QCOMPARE(formatElapsedTime(1.0), QStringLiteral("1.0"));
    QCOMPARE(formatElapsedTime(5.7), QStringLiteral("5.7"));
    QCOMPARE(formatElapsedTime(45.3), QStringLiteral("45.3"));
}

void TestTimeFormat::testJustUnderMinute()
{
    // 59.9 stays in seconds-only form — no leading "0:".
    QCOMPARE(formatElapsedTime(59.9), QStringLiteral("59.9"));
}

void TestTimeFormat::testExactlyMinute()
{
    // The boundary lands in MM:SS.S form so it reads as a clock from
    // the very first second past the minute mark.
    QCOMPARE(formatElapsedTime(60.0), QStringLiteral("1:00.0"));
}

void TestTimeFormat::testJustOverMinute() { QCOMPARE(formatElapsedTime(60.5), QStringLiteral("1:00.5")); }

void TestTimeFormat::testTwoDigitMinutes()
{
    // 12:34.5 — two-digit minutes pad seconds to two digits, tenths at the end.
    QCOMPARE(formatElapsedTime(754.5), QStringLiteral("12:34.5"));
    QCOMPARE(formatElapsedTime(605.0), QStringLiteral("10:05.0"));
}

void TestTimeFormat::testJustUnderHour() { QCOMPARE(formatElapsedTime(3599.9), QStringLiteral("59:59.9")); }

void TestTimeFormat::testExactlyHour() { QCOMPARE(formatElapsedTime(3600.0), QStringLiteral("1:00:00.0")); }

void TestTimeFormat::testMultiHour()
{
    // 1:02:03.4 — sanity check on H:MM:SS.S layout.
    QCOMPARE(formatElapsedTime(3723.4), QStringLiteral("1:02:03.4"));
    // 12:34:05.0 — two-digit hours flow naturally; minutes/seconds always
    // padded to two digits.
    QCOMPARE(formatElapsedTime(45245.0), QStringLiteral("12:34:05.0"));
}

void TestTimeFormat::testTenthsRoundUpAcrossSecondBoundary()
{
    // 9.97 rounds to 10.0 — must show "10.0", not "9.10".
    QCOMPARE(formatElapsedTime(9.97), QStringLiteral("10.0"));
    // 59.97 rounds to 60.0 — must cross the minute boundary, not show
    // "60.0" or "59.10".
    QCOMPARE(formatElapsedTime(59.97), QStringLiteral("1:00.0"));
}

void TestTimeFormat::testTenthsRoundUpAcrossMinuteBoundary()
{
    // 1:59.97 rounds to 2:00.0 — must propagate through both seconds and
    // minutes carry, not show "1:60.0".
    QCOMPARE(formatElapsedTime(119.97), QStringLiteral("2:00.0"));
    // 59:59.97 rounds to 1:00:00.0 — full carry through into the hours
    // bucket.
    QCOMPARE(formatElapsedTime(3599.97), QStringLiteral("1:00:00.0"));
}

void TestTimeFormat::testNegativeRendersAsZero()
{
    // Defensive: a degenerate elapsed-time should never reach the user as
    // "-1.0" or similar. Clamp to "0.0".
    QCOMPARE(formatElapsedTime(-1.0), QStringLiteral("0.0"));
    QCOMPARE(formatElapsedTime(-0.000001), QStringLiteral("0.0"));
}

void TestTimeFormat::testNonFiniteRendersAsZero()
{
    QCOMPARE(formatElapsedTime(std::numeric_limits<double>::quiet_NaN()), QStringLiteral("0.0"));
    QCOMPARE(formatElapsedTime(std::numeric_limits<double>::infinity()), QStringLiteral("0.0"));
    QCOMPARE(formatElapsedTime(-std::numeric_limits<double>::infinity()), QStringLiteral("0.0"));
}

void TestTimeFormat::testFractionFormattingPreservesTrailingZero()
{
    // The display always shows exactly one decimal — "5.0" not "5", and
    // "1:00.0" not "1:00". Pin so a future format-string refactor doesn't
    // drop the trailing tenth.
    QCOMPARE(formatElapsedTime(5.0), QStringLiteral("5.0"));
    QCOMPARE(formatElapsedTime(60.0), QStringLiteral("1:00.0"));
    QCOMPARE(formatElapsedTime(3600.0), QStringLiteral("1:00:00.0"));
}

QTEST_APPLESS_MAIN(TestTimeFormat)
#include "tst_time_format.moc"
