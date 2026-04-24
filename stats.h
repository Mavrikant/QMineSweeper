#ifndef STATS_H
#define STATS_H

#include <QDate>
#include <QString>

#include <cstdint>

// Per-difficulty lifetime statistics, persisted in QSettings under the
// stats/<DifficultyName>/{played,won,best_seconds,best_date,
// best_noflag_seconds,best_noflag_date} tree. Best-time is stored as seconds
// (double); 0 means "no win recorded yet". Best-date is the calendar date
// (ISO 8601) on which the current best-time run was completed; invalid/empty
// when no win has been recorded. The no-flag pair tracks the fastest win in
// which the user did not place a single flag — a separate speedrun bracket
// for advanced players who memorise/calculate mine positions instead.
namespace Stats
{
struct Record
{
    std::uint32_t played{0};
    std::uint32_t won{0};
    double bestSeconds{0.0};       // 0 == no record yet
    QDate bestDate{};              // invalid when no record yet
    double bestNoflagSeconds{0.0}; // 0 == no no-flag record yet
    QDate bestNoflagDate{};        // invalid when no no-flag record yet
};

[[nodiscard]] Record load(const QString &difficultyName);
void save(const QString &difficultyName, const Record &record);
void reset(const QString &difficultyName);
void resetAll();

// Convenience: increment Played (never mutates bestSeconds). Used on loss.
void recordLoss(const QString &difficultyName);

// Convenience: increment Played + Won, and update bestSeconds if the
// run was faster (or no prior record). Returns true iff a new record
// was set — so the caller can show a "New record!" flair.
// `onDate` lets callers (primarily tests) inject the date stamped onto
// a new best; in production it defaults to today.
bool recordWin(const QString &difficultyName, double seconds, const QDate &onDate = QDate::currentDate());

// Update only the no-flag best for a difficulty if the run was faster (or
// no prior no-flag record). Does NOT touch played/won/bestSeconds — call
// `recordWin` first for those, then call this when the run was completed
// without placing any flag. Returns true iff a new no-flag record was set.
bool recordNoflagBest(const QString &difficultyName, double seconds, const QDate &onDate = QDate::currentDate());
} // namespace Stats

#endif // STATS_H
