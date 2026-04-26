#include "../safe_percent_format.h"

#include <QLocale>
#include <QtTest>

class TestSafePercentFormat : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void testZeroRendersAsEmDash();
    void testNegativeRendersAsEmDash();
    void testOver100RendersAsEmDash();
    void testPositiveWithoutDateRendersPercentSuffix();
    void testPositiveWithDateAppendsLocaleShortDate();
    void test100PercentBoundary();
    void test1PercentBoundary();
    void testInvalidDateFallsBackToValueOnly();
    void testTwoSpaceSeparatorBeforeDate();
};

void TestSafePercentFormat::initTestCase()
{
    // Pin a deterministic locale so the date-formatted assertions don't
    // depend on the host locale running the suite.
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
}

void TestSafePercentFormat::testZeroRendersAsEmDash()
{
    // 0 is the persistence-layer "no record yet" sentinel — explicit gate
    // mirrors the bestFlagAccuracyPercent / bestBvPerSecond cells.
    QCOMPARE(formatSafePercentCell(0, QDate{2026, 4, 25}), QStringLiteral("—"));
}

void TestSafePercentFormat::testNegativeRendersAsEmDash()
{
    // Defence in depth — a future caller passing a sign-flipped int (e.g.
    // arithmetic bug at the call site) must not produce a negative percent
    // string. The persistence layer clamps to [0, 100] before saving, so
    // this guards only against future call-site bugs.
    QCOMPARE(formatSafePercentCell(-5, QDate{2026, 4, 25}), QStringLiteral("—"));
}

void TestSafePercentFormat::testOver100RendersAsEmDash()
{
    // Defence in depth — a value > 100 is impossible from the persistence
    // layer (clamps to 100), but the format helper rejects it cleanly
    // rather than rendering "150%" garbage.
    QCOMPARE(formatSafePercentCell(150, QDate{2026, 4, 25}), QStringLiteral("—"));
    QCOMPARE(formatSafePercentCell(101, QDate{2026, 4, 25}), QStringLiteral("—"));
}

void TestSafePercentFormat::testPositiveWithoutDateRendersPercentSuffix()
{
    // Invalid (default-constructed) QDate -> no parenthesised suffix.
    QCOMPARE(formatSafePercentCell(50, QDate{}), QStringLiteral("50%"));
    QCOMPARE(formatSafePercentCell(75, QDate{}), QStringLiteral("75%"));
    QCOMPARE(formatSafePercentCell(33, QDate{}), QStringLiteral("33%"));
}

void TestSafePercentFormat::testPositiveWithDateAppendsLocaleShortDate()
{
    const QDate d{2026, 4, 25};
    const QString expectedDate = QLocale().toString(d, QLocale::ShortFormat);
    const QString got = formatSafePercentCell(80, d);
    QCOMPARE(got, QStringLiteral("80%  (") + expectedDate + QStringLiteral(")"));
}

void TestSafePercentFormat::test100PercentBoundary()
{
    // Boundary value: a 100% partial-clear is mathematically the player's
    // last-cell-before-victory loss; renders cleanly without overflowing
    // or collapsing to em-dash. The exclusive-101 guard above pins the
    // upper bound of the accept range.
    QCOMPARE(formatSafePercentCell(100, QDate{}), QStringLiteral("100%"));
    QCOMPARE(formatSafePercentCell(100, QDate{2026, 4, 25}).left(4), QStringLiteral("100%"));
}

void TestSafePercentFormat::test1PercentBoundary()
{
    // Lower-bound boundary: a single-cell-cleared loss rounds to a tiny but
    // renderable value. The persistence layer's `safePercent > 0` gate
    // ensures sub-1% rounds-to-zero losses never reach this helper.
    QCOMPARE(formatSafePercentCell(1, QDate{}), QStringLiteral("1%"));
}

void TestSafePercentFormat::testInvalidDateFallsBackToValueOnly()
{
    // QDate::fromString("", Qt::ISODate) returns an invalid date — the
    // exact path the persistence layer hits when no record has ever
    // been saved. The cell should drop the parenthesised suffix.
    QCOMPARE(formatSafePercentCell(60, QDate::fromString(QStringLiteral(""), Qt::ISODate)), QStringLiteral("60%"));
}

void TestSafePercentFormat::testTwoSpaceSeparatorBeforeDate()
{
    // Two-space separator between value and parenthesised date matches the
    // Best-time / Best-(no flag) / Streak / Best-3BV/s / Best-flag-accuracy
    // cells. Pin it so a future format-string change can't silently
    // de-align the column.
    const QString got = formatSafePercentCell(70, QDate{2026, 4, 25});
    QVERIFY(got.contains(QStringLiteral("  (")));
}

QTEST_MAIN(TestSafePercentFormat)
#include "tst_safe_percent_format.moc"
