#ifndef BV_PER_SECOND_FORMAT_H
#define BV_PER_SECOND_FORMAT_H

#include <QDate>
#include <QLocale>
#include <QString>

// Format a per-difficulty `Best 3BV/s` cell for the Statistics dialog.
//
//   bvPerSecond <= 0        -> "—"           (no record yet — also the
//                                            sentinel returned for
//                                            non-finite/negative input)
//   bvPerSecond > 0, no date -> "X.YZ"
//   bvPerSecond > 0, date    -> "X.YZ  (LL.LL.YYYY)"
//
// The two-decimal rate matches the live win-dialog
// `3BV: %1 · 3BV/s: %2` format and the `⚡ New best 3BV/s!` flair, so
// the persisted lifetime record reads identically to the celebration
// the user just saw. Date, when present, is rendered in the active
// QLocale's short format and inlined in parentheses — mirroring the
// Best-time / Best-(no flag) / Streak cells.
inline QString formatBvPerSecondCell(double bvPerSecond, const QDate &date)
{
    if (!(bvPerSecond > 0.0))
    {
        return QStringLiteral("—");
    }
    QString s = QString::asprintf("%.2f", bvPerSecond);
    if (date.isValid())
    {
        s += QStringLiteral("  (") + QLocale().toString(date, QLocale::ShortFormat) + QStringLiteral(")");
    }
    return s;
}

#endif // BV_PER_SECOND_FORMAT_H
