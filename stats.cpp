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
}

void resetAll()
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("stats"));
    settings.remove(QString{}); // wipe the whole group
    settings.endGroup();
}

LossOutcome recordLoss(const QString &difficultyName, int safePercent, const QDate &onDate)
{
    Record r = load(difficultyName);
    ++r.played;
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
    save(difficultyName, r);
    return LossOutcome{newBestSafePercent};
}

WinOutcome recordWin(const QString &difficultyName, double seconds, const QDate &onDate, double bvPerSecond)
{
    Record r = load(difficultyName);
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
    save(difficultyName, r);
    return WinOutcome{newBestTime && seconds > 0.0, r.currentStreak, newBestStreak, newBestBvPerSecond};
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
