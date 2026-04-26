#include "stats.h"

#include <QSettings>

namespace
{
QString key(const QString &difficultyName, const QString &field) { return QStringLiteral("stats/%1/%2").arg(difficultyName, field); }
} // namespace

namespace Stats
{
Record load(const QString &difficultyName)
{
    QSettings settings;
    Record r;
    r.played = settings.value(key(difficultyName, QStringLiteral("played")), 0u).toUInt();
    r.won = settings.value(key(difficultyName, QStringLiteral("won")), 0u).toUInt();
    r.bestSeconds = settings.value(key(difficultyName, QStringLiteral("best_seconds")), 0.0).toDouble();
    const QString dateStr = settings.value(key(difficultyName, QStringLiteral("best_date")), QString{}).toString();
    r.bestDate = dateStr.isEmpty() ? QDate{} : QDate::fromString(dateStr, Qt::ISODate);
    r.bestNoflagSeconds = settings.value(key(difficultyName, QStringLiteral("best_noflag_seconds")), 0.0).toDouble();
    const QString noflagDateStr = settings.value(key(difficultyName, QStringLiteral("best_noflag_date")), QString{}).toString();
    r.bestNoflagDate = noflagDateStr.isEmpty() ? QDate{} : QDate::fromString(noflagDateStr, Qt::ISODate);
    r.currentStreak = settings.value(key(difficultyName, QStringLiteral("streak_current")), 0u).toUInt();
    r.bestStreak = settings.value(key(difficultyName, QStringLiteral("streak_best")), 0u).toUInt();
    const QString streakDateStr = settings.value(key(difficultyName, QStringLiteral("streak_best_date")), QString{}).toString();
    r.bestStreakDate = streakDateStr.isEmpty() ? QDate{} : QDate::fromString(streakDateStr, Qt::ISODate);
    r.bestSafePercent = settings.value(key(difficultyName, QStringLiteral("best_safe_percent")), 0u).toUInt();
    const QString safePercentDateStr = settings.value(key(difficultyName, QStringLiteral("best_safe_percent_date")), QString{}).toString();
    r.bestSafePercentDate = safePercentDateStr.isEmpty() ? QDate{} : QDate::fromString(safePercentDateStr, Qt::ISODate);
    r.bestBvPerSecond = settings.value(key(difficultyName, QStringLiteral("best_bv_per_second")), 0.0).toDouble();
    const QString bvDateStr = settings.value(key(difficultyName, QStringLiteral("best_bv_per_second_date")), QString{}).toString();
    r.bestBvPerSecondDate = bvDateStr.isEmpty() ? QDate{} : QDate::fromString(bvDateStr, Qt::ISODate);
    r.bestFlagAccuracyPercent = settings.value(key(difficultyName, QStringLiteral("best_flag_accuracy_percent")), 0u).toUInt();
    const QString flagAccDateStr = settings.value(key(difficultyName, QStringLiteral("best_flag_accuracy_date")), QString{}).toString();
    r.bestFlagAccuracyDate = flagAccDateStr.isEmpty() ? QDate{} : QDate::fromString(flagAccDateStr, Qt::ISODate);
    r.totalSecondsWon = settings.value(key(difficultyName, QStringLiteral("total_seconds_won")), 0.0).toDouble();
    const QString lastWinDateStr = settings.value(key(difficultyName, QStringLiteral("last_win_date")), QString{}).toString();
    r.lastWinDate = lastWinDateStr.isEmpty() ? QDate{} : QDate::fromString(lastWinDateStr, Qt::ISODate);
    return r;
}

void save(const QString &difficultyName, const Record &r)
{
    QSettings settings;
    settings.setValue(key(difficultyName, QStringLiteral("played")), r.played);
    settings.setValue(key(difficultyName, QStringLiteral("won")), r.won);
    settings.setValue(key(difficultyName, QStringLiteral("best_seconds")), r.bestSeconds);
    if (r.bestDate.isValid())
    {
        settings.setValue(key(difficultyName, QStringLiteral("best_date")), r.bestDate.toString(Qt::ISODate));
    }
    else
    {
        settings.remove(key(difficultyName, QStringLiteral("best_date")));
    }
    settings.setValue(key(difficultyName, QStringLiteral("best_noflag_seconds")), r.bestNoflagSeconds);
    if (r.bestNoflagDate.isValid())
    {
        settings.setValue(key(difficultyName, QStringLiteral("best_noflag_date")), r.bestNoflagDate.toString(Qt::ISODate));
    }
    else
    {
        settings.remove(key(difficultyName, QStringLiteral("best_noflag_date")));
    }
    settings.setValue(key(difficultyName, QStringLiteral("streak_current")), r.currentStreak);
    settings.setValue(key(difficultyName, QStringLiteral("streak_best")), r.bestStreak);
    if (r.bestStreakDate.isValid())
    {
        settings.setValue(key(difficultyName, QStringLiteral("streak_best_date")), r.bestStreakDate.toString(Qt::ISODate));
    }
    else
    {
        settings.remove(key(difficultyName, QStringLiteral("streak_best_date")));
    }
    settings.setValue(key(difficultyName, QStringLiteral("best_safe_percent")), r.bestSafePercent);
    if (r.bestSafePercentDate.isValid())
    {
        settings.setValue(key(difficultyName, QStringLiteral("best_safe_percent_date")), r.bestSafePercentDate.toString(Qt::ISODate));
    }
    else
    {
        settings.remove(key(difficultyName, QStringLiteral("best_safe_percent_date")));
    }
    settings.setValue(key(difficultyName, QStringLiteral("best_bv_per_second")), r.bestBvPerSecond);
    if (r.bestBvPerSecondDate.isValid())
    {
        settings.setValue(key(difficultyName, QStringLiteral("best_bv_per_second_date")), r.bestBvPerSecondDate.toString(Qt::ISODate));
    }
    else
    {
        settings.remove(key(difficultyName, QStringLiteral("best_bv_per_second_date")));
    }
    settings.setValue(key(difficultyName, QStringLiteral("best_flag_accuracy_percent")), r.bestFlagAccuracyPercent);
    if (r.bestFlagAccuracyDate.isValid())
    {
        settings.setValue(key(difficultyName, QStringLiteral("best_flag_accuracy_date")), r.bestFlagAccuracyDate.toString(Qt::ISODate));
    }
    else
    {
        settings.remove(key(difficultyName, QStringLiteral("best_flag_accuracy_date")));
    }
    settings.setValue(key(difficultyName, QStringLiteral("total_seconds_won")), r.totalSecondsWon);
    if (r.lastWinDate.isValid())
    {
        settings.setValue(key(difficultyName, QStringLiteral("last_win_date")), r.lastWinDate.toString(Qt::ISODate));
    }
    else
    {
        settings.remove(key(difficultyName, QStringLiteral("last_win_date")));
    }
}

void reset(const QString &difficultyName)
{
    QSettings settings;
    settings.remove(key(difficultyName, QStringLiteral("played")));
    settings.remove(key(difficultyName, QStringLiteral("won")));
    settings.remove(key(difficultyName, QStringLiteral("best_seconds")));
    settings.remove(key(difficultyName, QStringLiteral("best_date")));
    settings.remove(key(difficultyName, QStringLiteral("best_noflag_seconds")));
    settings.remove(key(difficultyName, QStringLiteral("best_noflag_date")));
    settings.remove(key(difficultyName, QStringLiteral("streak_current")));
    settings.remove(key(difficultyName, QStringLiteral("streak_best")));
    settings.remove(key(difficultyName, QStringLiteral("streak_best_date")));
    settings.remove(key(difficultyName, QStringLiteral("best_safe_percent")));
    settings.remove(key(difficultyName, QStringLiteral("best_safe_percent_date")));
    settings.remove(key(difficultyName, QStringLiteral("best_bv_per_second")));
    settings.remove(key(difficultyName, QStringLiteral("best_bv_per_second_date")));
    settings.remove(key(difficultyName, QStringLiteral("best_flag_accuracy_percent")));
    settings.remove(key(difficultyName, QStringLiteral("best_flag_accuracy_date")));
    settings.remove(key(difficultyName, QStringLiteral("total_seconds_won")));
    settings.remove(key(difficultyName, QStringLiteral("last_win_date")));
}

void resetAll()
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("stats"));
    settings.remove(QString{}); // wipe the whole group
    settings.endGroup();
}

LossOutcome recordLoss(const QString &difficultyName, int safePercent, int flagAccuracyPercent, const QDate &onDate)
{
    Record r = load(difficultyName);
    ++r.played;
    // Capture the streak *before* zeroing so the loss dialog can surface a
    // "💔 Streak ended at %1" line for streaks of 2+. The line itself is gated
    // on >=2 in the dialog; the field is reported regardless so callers /
    // tests can read the raw value.
    const std::uint32_t priorStreak = r.currentStreak;
    r.currentStreak = 0;
    bool newBestSafePercent = false;
    if (safePercent > 0)
    {
        const std::uint32_t clamped = static_cast<std::uint32_t>(safePercent > 100 ? 100 : safePercent);
        if (clamped > r.bestSafePercent)
        {
            r.bestSafePercent = clamped;
            r.bestSafePercentDate = onDate;
            newBestSafePercent = true;
        }
    }
    bool newBestFlagAccuracyPercent = false;
    if (flagAccuracyPercent > 0)
    {
        const std::uint32_t clampedAcc = static_cast<std::uint32_t>(flagAccuracyPercent > 100 ? 100 : flagAccuracyPercent);
        if (clampedAcc > r.bestFlagAccuracyPercent)
        {
            r.bestFlagAccuracyPercent = clampedAcc;
            r.bestFlagAccuracyDate = onDate;
            newBestFlagAccuracyPercent = true;
        }
    }
    save(difficultyName, r);
    return LossOutcome{newBestSafePercent, newBestFlagAccuracyPercent, priorStreak};
}

WinOutcome recordWin(const QString &difficultyName, double seconds, const QDate &onDate, double bvPerSecond)
{
    Record r = load(difficultyName);
    // Capture the pre-mutation best so the win dialog can render the
    // `🏆 New record!  …  (prev %1)` companion with the value being beaten.
    // Mirror of the v1.39 `priorStreak` capture in `recordLoss`: snapshot
    // before any field is touched, return both pre- and post- values in the
    // outcome. For a player with no prior win this stays at the 0.0 sentinel,
    // which the dialog uses as a "no previous record to anchor against" gate.
    const double priorBestSeconds = r.bestSeconds;
    ++r.played;
    ++r.won;
    const bool newBestTime = (r.bestSeconds <= 0.0) || (seconds > 0.0 && seconds < r.bestSeconds);
    if (newBestTime && seconds > 0.0)
    {
        r.bestSeconds = seconds;
        r.bestDate = onDate;
    }
    ++r.currentStreak;
    const bool newBestStreak = r.currentStreak > r.bestStreak;
    if (newBestStreak)
    {
        r.bestStreak = r.currentStreak;
        r.bestStreakDate = onDate;
    }
    bool newBestBvPerSecond = false;
    if (bvPerSecond > 0.0 && bvPerSecond > r.bestBvPerSecond)
    {
        r.bestBvPerSecond = bvPerSecond;
        r.bestBvPerSecondDate = onDate;
        newBestBvPerSecond = true;
    }
    // Lifetime mean-of-winning-times accumulator. Same > 0.0 gate as the
    // bestSeconds update path so a sub-tick test win never poisons the divisor.
    if (seconds > 0.0)
    {
        r.totalSecondsWon += seconds;
    }
    // Most-recent-win timestamp — overwritten unconditionally on every counted
    // win, even sub-tick (the date is meaningful regardless of duration). Drives
    // the loss-dialog "Last win: %1" line on the *next* loss for this difficulty.
    r.lastWinDate = onDate;
    save(difficultyName, r);
    // averageSecondsAfter is the post-update mean, computed from the same
    // numerator (totalSecondsWon) and denominator (won) the next load() will
    // see. Guarded against 0/0 even though `won` was just incremented — the
    // accumulator may still be 0.0 if every win so far was sub-tick.
    const double averageSecondsAfter = (r.won > 0 && r.totalSecondsWon > 0.0) ? (r.totalSecondsWon / r.won) : 0.0;
    // bestSecondsAfter mirrors r.bestSeconds post-newBestTime mutation. Same
    // value the next Stats::load() will see; the call site reads it for the
    // win-dialog `Average: %1 (best %2)` companion line.
    return WinOutcome{newBestTime && seconds > 0.0, r.currentStreak, newBestStreak, newBestBvPerSecond, r.won, averageSecondsAfter, r.bestSeconds, priorBestSeconds};
}

bool recordNoflagBest(const QString &difficultyName, double seconds, const QDate &onDate)
{
    if (seconds <= 0.0)
    {
        return false;
    }
    Record r = load(difficultyName);
    const bool newRecord = (r.bestNoflagSeconds <= 0.0) || (seconds < r.bestNoflagSeconds);
    if (!newRecord)
    {
        return false;
    }
    r.bestNoflagSeconds = seconds;
    r.bestNoflagDate = onDate;
    save(difficultyName, r);
    return true;
}
} // namespace Stats
