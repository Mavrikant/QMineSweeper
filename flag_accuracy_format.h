#ifndef FLAG_ACCURACY_FORMAT_H
#define FLAG_ACCURACY_FORMAT_H

#include <QDate>
#include <QLocale>
#include <QString>

#include <cstdint>

// Format a per-difficulty `Best flag accuracy` cell for the Statistics dialog.
//
//   percent == 0           -> "—"           (no record yet — also returned
//                                            for any input outside (0, 100])
//   percent > 0, no date   -> "X%"
//   percent > 0, date      -> "X%  (LL.LL.YYYY)"
//
// The integer-percent format matches the rounded `correctFlags / flagsPlaced`
// metric used by the persistence layer (see `Stats::recordLoss`). Date, when
// present, is rendered in the active QLocale's short format and inlined in
// parentheses — mirroring the Best-time / Best-(no flag) / Streak / Best-3BV/s
// cells. Out-of-range percents (>100 or negative) collapse to "—" rather than
// rendering nonsense; the persistence layer already clamps to [0, 100], so in
// production the guard catches only future call-site arithmetic bugs.
inline QString formatFlagAccuracyCell(int percent, const QDate &date)
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

#endif // FLAG_ACCURACY_FORMAT_H
