#ifndef STATS_H
#define STATS_H

#include <QDate>
#include <QString>

#include <cstdint>

// Per-difficulty lifetime statistics, persisted in QSettings under the
// stats/<DifficultyName>/{played,won,best_seconds,best_date,
// best_noflag_seconds,best_noflag_date,streak_current,streak_best,
// streak_best_date} tree. Best-time is stored as seconds (double); 0 means
// "no win recorded yet". Best-date is the calendar date (ISO 8601) on which
// the current best-time run was completed; invalid/empty when no win has been
// recorded. The no-flag pair tracks the fastest win in which the user did not
// place a single flag — a separate speedrun bracket for advanced players who
// memorise/calculate mine positions instead. Streak is the per-difficulty
// consecutive-wins counter: `currentStreak` resets to 0 on every loss,
// `bestStreak` is the lifetime high-water mark with `bestStreakDate` the day
// the high-water mark was set or last extended.
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
    std::uint32_t currentStreak{0};
    std::uint32_t bestStreak{0};
    QDate bestStreakDate{}; // invalid when no streak recorded yet
};

// Outcome of a recordWin call. `newRecord` matches the prior boolean return:
// true iff `seconds` set a new best-time. `newBestStreak` is true iff the win
// pushed `currentStreak` past the prior `bestStreak` (including the very first
// win, when both transition 0 → 1).
struct WinOutcome
{
    bool newRecord{false};
    std::uint32_t currentStreak{0};
    bool newBestStreak{false};

    // Lets `QVERIFY(recordWin(...))` and `if (recordWin(...))` keep their
    // pre-WinOutcome bool semantics (true == new best-time set). Explicit so
    // accidental assignment to `bool` still requires `.newRecord`.
    explicit operator bool() const noexcept { return newRecord; }
};

[[nodiscard]] Record load(const QString &difficultyName);
void save(const QString &difficultyName, const Record &record);
void reset(const QString &difficultyName);
void resetAll();

// Convenience: increment Played (never mutates bestSeconds) and zero
// currentStreak. Used on loss.
void recordLoss(const QString &difficultyName);

// Convenience: increment Played + Won, update bestSeconds if the run was
// faster (or no prior record), increment currentStreak, and roll bestStreak
// forward when current crosses it. Returns the full outcome so the caller can
// surface "New record!" / "Best streak!" flair on the win dialog.
// `onDate` lets callers (primarily tests) inject the date stamped onto a new
// best (best-time and best-streak share the date); in production it defaults
// to today.
WinOutcome recordWin(const QString &difficultyName, double seconds, const QDate &onDate = QDate::currentDate());

// Update only the no-flag best for a difficulty if the run was faster (or
// no prior no-flag record). Does NOT touch played/won/bestSeconds — call
// `recordWin` first for those, then call this when the run was completed
// without placing any flag. Returns true iff a new no-flag record was set.
bool recordNoflagBest(const QString &difficultyName, double seconds, const QDate &onDate = QDate::currentDate());
} // namespace Stats

#endif // STATS_H
