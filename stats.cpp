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
}

void reset(const QString &difficultyName)
{
    QSettings settings;
    settings.remove(key(difficultyName, QStringLiteral("played")));
    settings.remove(key(difficultyName, QStringLiteral("won")));
    settings.remove(key(difficultyName, QStringLiteral("best_seconds")));
    settings.remove(key(difficultyName, QStringLiteral("best_date")));
}

void resetAll()
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("stats"));
    settings.remove(QString{}); // wipe the whole group
    settings.endGroup();
}

void recordLoss(const QString &difficultyName)
{
    Record r = load(difficultyName);
    ++r.played;
    save(difficultyName, r);
}

bool recordWin(const QString &difficultyName, double seconds, const QDate &onDate)
{
    Record r = load(difficultyName);
    ++r.played;
    ++r.won;
    const bool newRecord = (r.bestSeconds <= 0.0) || (seconds > 0.0 && seconds < r.bestSeconds);
    if (newRecord && seconds > 0.0)
    {
        r.bestSeconds = seconds;
        r.bestDate = onDate;
    }
    save(difficultyName, r);
    return newRecord && seconds > 0.0;
}
} // namespace Stats
