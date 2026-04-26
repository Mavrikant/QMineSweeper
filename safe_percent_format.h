#ifndef SAFE_PERCENT_FORMAT_H
#define SAFE_PERCENT_FORMAT_H

#include <QDate>
#include <QLocale>
#include <QString>

#include <cstdint>

// Format a per-difficulty `Best partial` cell for the Statistics dialog.
//
//   percent == 0           -> "—"           (no record yet — also returned
//                                            for any input outside (0, 100])
//   percent > 0, no date   -> "X%"
//   percent > 0, date      -> "X%  (LL.LL.YYYY)"
//
// The integer-percent format matches the rounded board-coverage metric used by
// the persistence layer (see `Stats::recordLoss`'s `safePercent` argument).
// Date, when present, is rendered in the active QLocale's short format and
// inlined in parentheses — mirroring the Best-time / Best-(no flag) / Streak
// / Best-3BV/s / Best-flag-accuracy cells. Out-of-range percents (>100 or
// negative) collapse to "—" rather than rendering nonsense; the persistence
// layer already clamps to [0, 100], so in production the guard catches only
// future call-site arithmetic bugs. Mirrors `formatFlagAccuracyCell` byte-for-
// byte today; kept separate so a future per-axis format tweak (e.g. one-decimal
// safe-percent vs. integer flag-accuracy) can land without untangling a shared
// helper.
inline QString formatSafePercentCell(int percent, const QDate &date)
{
    if (percent <= 0 || percent > 100)
    {
        return QStringLiteral("—");
    }
    QString s = QStringLiteral("%1%").arg(percent);
    if (date.isValid())
    {
        s += QStringLiteral("  (") + QLocale().toString(date, QLocale::ShortFormat) + QStringLiteral(")");
    }
    return s;
}

#endif // SAFE_PERCENT_FORMAT_H
