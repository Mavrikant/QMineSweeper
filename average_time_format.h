#ifndef AVERAGE_TIME_FORMAT_H
#define AVERAGE_TIME_FORMAT_H

#include "time_format.h"

#include <QString>

#include <cstdint>

// Format a per-difficulty `Average` cell for the Statistics dialog.
//
//   won == 0                  -> "—"   (no wins recorded — also the
//                                       divide-by-zero guard)
//   totalSecondsWon <= 0.0    -> "—"   (every counted win was sub-tick;
//                                       the mean is mathematically 0.0
//                                       but not informative — surface
//                                       "—" instead of "0.0")
//   else                      -> formatElapsedTime(totalSecondsWon / won)
//
// The duration-aware clock format matches the live timer label and the
// win-dialog `Average: %1` line, so a player who reads the Average cell
// after winning sees the same number in the same format the celebration
// just showed. No date suffix — Average is a lifetime mean across many
// runs, so pinning it to a single date would be misleading (unlike
// Best-time / Best-3BV/s / Streak / Last-win, all of which anchor to a
// specific run).
inline QString formatAverageCell(double totalSecondsWon, std::uint32_t won)
{
    if (won == 0 || totalSecondsWon <= 0.0)
    {
        return QStringLiteral("—");
    }
    return formatElapsedTime(totalSecondsWon / static_cast<double>(won));
}

#endif // AVERAGE_TIME_FORMAT_H
