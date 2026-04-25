#ifndef TIME_FORMAT_H
#define TIME_FORMAT_H

#include <QString>

#include <cmath>

// Format a non-negative elapsed-time in seconds as a duration-aware
// clock string with one decimal place:
//
//   < 60 s          -> "S.S"      (e.g. "5.3", "45.0")
//   60 s..< 1 h     -> "M:SS.S"   (e.g. "1:30.5", "59:09.7")
//   >= 1 h          -> "H:MM:SS.S"(e.g. "1:00:00.0", "12:34:05.0")
//
// Negative or non-finite inputs render as "0.0" so a degenerate clock
// reading never reaches the user.
inline QString formatElapsedTime(double seconds)
{
    if (!std::isfinite(seconds) || seconds < 0.0)
    {
        return QStringLiteral("0.0");
    }

    // Round to tenths once so the integer breakdown agrees with the
    // displayed decimal — without this, 59.97 would show "59.9" but be
    // bucketed as < 60 while the printf-rounded "60.0" would land in
    // the seconds-only branch.
    const double rounded = std::round(seconds * 10.0) / 10.0;

    if (rounded < 60.0)
    {
        return QString::asprintf("%.1f", rounded);
    }

    const int totalTenths = static_cast<int>(std::llround(rounded * 10.0));
    const int totalSecondsInt = totalTenths / 10;
    const int tenths = totalTenths % 10;
    const int hours = totalSecondsInt / 3600;
    const int minutes = (totalSecondsInt % 3600) / 60;
    const int seconds_i = totalSecondsInt % 60;

    if (hours == 0)
    {
        return QString::asprintf("%d:%02d.%d", minutes, seconds_i, tenths);
    }
    return QString::asprintf("%d:%02d:%02d.%d", hours, minutes, seconds_i, tenths);
}

#endif // TIME_FORMAT_H
