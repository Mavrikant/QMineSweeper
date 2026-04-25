#include "../flag_accuracy_format.h"

#include <QLocale>
#include <QtTest>

class TestFlagAccuracyFormat : public QObject
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

void TestFlagAccuracyFormat::initTestCase()
{
    // Pin a deterministic locale so the date-formatted assertions don't
    // depend on the host locale running the suite.
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
}

void TestFlagAccuracyFormat::testZeroRendersAsEmDash()
{
    // 0 is the persistence-layer "no record yet" sentinel — explicit gate
    // mirrors the bestSafePercent / bestBvPerSecond cells.
    QCOMPARE(formatFlagAccuracyCell(0, QDate{2026, 4, 25}), QStringLiteral("—"));
}

void TestFlagAccuracyFormat::testNegativeRendersAsEmDash()
{
    // Defence in depth — a future caller passing a sign-flipped int (e.g.
    // arithmetic bug at the call site) must not produce a negative percent
    // string. The persistence layer clamps to [0, 100] before saving, so
    // this guards only against future call-site bugs.
    QCOMPARE(formatFlagAccuracyCell(-5, QDate{2026, 4, 25}), QStringLiteral("—"));
}

void TestFlagAccuracyFormat::testOver100RendersAsEmDash()
{
    // Defence in depth — a value > 100 is impossible from the persistence
    // layer (clamps to 100), but the format helper rejects it cleanly
    // rather than rendering "150%" garbage.
    QCOMPARE(formatFlagAccuracyCell(150, QDate{2026, 4, 25}), QStringLiteral("—"));
    QCOMPARE(formatFlagAccuracyCell(101, QDate{2026, 4, 25}), QStringLiteral("—"));
}

void TestFlagAccuracyFormat::testPositiveWithoutDateRendersPercentSuffix()
{
    // Invalid (default-constructed) QDate -> no parenthesised suffix.
    QCOMPARE(formatFlagAccuracyCell(50, QDate{}), QStringLiteral("50%"));
    QCOMPARE(formatFlagAccuracyCell(75, QDate{}), QStringLiteral("75%"));
    QCOMPARE(formatFlagAccuracyCell(33, QDate{}), QStringLiteral("33%"));
}

void TestFlagAccuracyFormat::testPositiveWithDateAppendsLocaleShortDate()
{
    const QDate d{2026, 4, 25};
    const QString expectedDate = QLocale().toString(d, QLocale::ShortFormat);
    const QString got = formatFlagAccuracyCell(80, d);
    QCOMPARE(got, QStringLiteral("80%  (") + expectedDate + QStringLiteral(")"));
}

void TestFlagAccuracyFormat::test100PercentBoundary()
{
    // Boundary value: a perfect-flagger run renders "100%" without
    // overflowing or collapsing to em-dash. The exclusive-101 guard above
    // pins the upper bound of the accept range.
    QCOMPARE(formatFlagAccuracyCell(100, QDate{}), QStringLiteral("100%"));
    QCOMPARE(formatFlagAccuracyCell(100, QDate{2026, 4, 25}).left(4), QStringLiteral("100%"));
}

void TestFlagAccuracyFormat::test1PercentBoundary()
{
    // Lower-bound boundary: a 1-correct-flag-out-of-many run rounds to a
    // tiny but renderable value (e.g. 1 / 99 → 1%).
    QCOMPARE(formatFlagAccuracyCell(1, QDate{}), QStringLiteral("1%"));
}

void TestFlagAccuracyFormat::testInvalidDateFallsBackToValueOnly()
{
    // QDate::fromString("", Qt::ISODate) returns an invalid date — the
    // exact path the persistence layer hits when no record has ever
    // been saved. The cell should drop the parenthesised suffix.
    QCOMPARE(formatFlagAccuracyCell(60, QDate::fromString(QStringLiteral(""), Qt::ISODate)), QStringLiteral("60%"));
}

void TestFlagAccuracyFormat::testTwoSpaceSeparatorBeforeDate()
{
    // Two-space separator between value and parenthesised date matches the
    // Best-time / Best-(no flag) / Streak / Best-3BV/s cells. Pin it so a
    // future format-string change can't silently de-align the column.
    const QString got = formatFlagAccuracyCell(70, QDate{2026, 4, 25});
    QVERIFY(got.contains(QStringLiteral("  (")));
}

QTEST_MAIN(TestFlagAccuracyFormat)
#include "tst_flag_accuracy_format.moc"
