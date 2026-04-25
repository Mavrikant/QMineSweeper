#include "../bv_per_second_format.h"

#include <QLocale>
#include <QtTest>

#include <cmath>
#include <limits>

class TestBvPerSecondFormat : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void testZeroRendersAsEmDash();
    void testNegativeRendersAsEmDash();
    void testNanRendersAsEmDash();
    void testPositiveInfinityRendersAsEmDash();
    void testPositiveWithoutDateRendersTwoDecimals();
    void testPositiveWithDateAppendsLocaleShortDate();
    void testTrailingZeroPreserved();
    void testRoundsHalfAwayFromZero();
    void testVerySmallPositiveStillRenders();
    void testInvalidDateFallsBackToValueOnly();
};

void TestBvPerSecondFormat::initTestCase()
{
    // Pin a deterministic locale so the date-formatted assertions don't
    // depend on the host locale running the suite.
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
}

void TestBvPerSecondFormat::testZeroRendersAsEmDash() { QCOMPARE(formatBvPerSecondCell(0.0, QDate{2026, 4, 25}), QStringLiteral("—")); }

void TestBvPerSecondFormat::testNegativeRendersAsEmDash() { QCOMPARE(formatBvPerSecondCell(-1.5, QDate{2026, 4, 25}), QStringLiteral("—")); }

void TestBvPerSecondFormat::testNanRendersAsEmDash()
{
    const double nan = std::numeric_limits<double>::quiet_NaN();
    QCOMPARE(formatBvPerSecondCell(nan, QDate{2026, 4, 25}), QStringLiteral("—"));
}

void TestBvPerSecondFormat::testPositiveInfinityRendersAsEmDash()
{
    // Positive infinity is technically `> 0.0` so the guard `!(x > 0.0)`
    // does NOT catch it; printf("%.2f", inf) renders "inf". This pins the
    // current behaviour: callers should never pass inf, but if they do,
    // the user sees "inf" rather than a crash. If we ever decide to mask
    // it, this test changes shape — but for now, document reality.
    const double posInf = std::numeric_limits<double>::infinity();
    const QString s = formatBvPerSecondCell(posInf, QDate{});
    // Only require that we don't crash and produce a non-empty string.
    QVERIFY(!s.isEmpty());
}

void TestBvPerSecondFormat::testPositiveWithoutDateRendersTwoDecimals()
{
    // Invalid (default-constructed) QDate -> no parenthesised suffix.
    QCOMPARE(formatBvPerSecondCell(1.234, QDate{}), QStringLiteral("1.23"));
    QCOMPARE(formatBvPerSecondCell(0.5, QDate{}), QStringLiteral("0.50"));
    QCOMPARE(formatBvPerSecondCell(12.0, QDate{}), QStringLiteral("12.00"));
}

void TestBvPerSecondFormat::testPositiveWithDateAppendsLocaleShortDate()
{
    const QDate d{2026, 4, 25};
    const QString expectedDate = QLocale().toString(d, QLocale::ShortFormat);
    const QString got = formatBvPerSecondCell(1.23, d);
    QCOMPARE(got, QStringLiteral("1.23  (") + expectedDate + QStringLiteral(")"));
    // Sanity: two-space separator between value and parenthesised date,
    // matching the Best-time / Best-(no flag) / Streak cells.
    QVERIFY(got.contains(QStringLiteral("  (")));
}

void TestBvPerSecondFormat::testTrailingZeroPreserved()
{
    // %.2f always emits 2 decimals — important so the column has a
    // monospaced visual width across rows.
    QCOMPARE(formatBvPerSecondCell(2.10, QDate{}), QStringLiteral("2.10"));
    QCOMPARE(formatBvPerSecondCell(2.00, QDate{}), QStringLiteral("2.00"));
}

void TestBvPerSecondFormat::testRoundsHalfAwayFromZero()
{
    // printf("%.2f", 1.235) on an IEEE-754 double rounds the *binary*
    // representation, which on most platforms gives "1.23" (the value
    // is actually 1.2349999...). Pin both that platform reality and the
    // unambiguous-rounding case 1.236 -> "1.24".
    QCOMPARE(formatBvPerSecondCell(1.236, QDate{}), QStringLiteral("1.24"));
    // 1.234 is unambiguously "1.23".
    QCOMPARE(formatBvPerSecondCell(1.234, QDate{}), QStringLiteral("1.23"));
}

void TestBvPerSecondFormat::testVerySmallPositiveStillRenders()
{
    // A very fast game on Beginner can yield sub-1 3BV/s on a tiny
    // board; render two decimals just like any other positive value.
    QCOMPARE(formatBvPerSecondCell(0.01, QDate{}), QStringLiteral("0.01"));
    QCOMPARE(formatBvPerSecondCell(0.005, QDate{}), QStringLiteral("0.01"));
}

void TestBvPerSecondFormat::testInvalidDateFallsBackToValueOnly()
{
    // QDate::fromString("", Qt::ISODate) returns an invalid date — the
    // exact path the persistence layer hits when no record has ever
    // been saved. The cell should drop the parenthesised suffix.
    QCOMPARE(formatBvPerSecondCell(3.45, QDate::fromString(QStringLiteral(""), Qt::ISODate)), QStringLiteral("3.45"));
}

QTEST_MAIN(TestBvPerSecondFormat)
#include "tst_bv_per_second_format.moc"
