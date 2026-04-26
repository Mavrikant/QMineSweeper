#ifndef STATS_H
#define STATS_H

#include <QDate>
#include <QString>

#include <cstdint>

// Per-difficulty lifetime statistics, persisted in QSettings under the
// stats/<DifficultyName>/{played,won,best_seconds,best_date,
// best_noflag_seconds,best_noflag_date,streak_current,streak_best,
// streak_best_date,best_safe_percent,best_safe_percent_date,
// best_bv_per_second,best_bv_per_second_date,
// best_flag_accuracy_percent,best_flag_accuracy_date,
// total_seconds_won,last_win_date} tree.
// Best-time is stored as seconds (double); 0 means "no win recorded yet".
// Best-date is the calendar date (ISO 8601) on which the current best-time
// run was completed; invalid/empty when no win has been recorded. The
// no-flag pair tracks the fastest win in which the user did not place a
// single flag — a separate speedrun bracket for advanced players who
// memorise/calculate mine positions instead. Streak is the per-difficulty
// consecutive-wins counter: `currentStreak` resets to 0 on every loss,
// `bestStreak` is the lifetime high-water mark with `bestStreakDate` the
// day the high-water mark was set or last extended. The
// best-safe-percent pair is the highest percentage of the board ever
// cleared on a *loss* in this difficulty — surfaced in the Stats dialog
// only when no win has been recorded yet, as a "partial-clear hall of
// fame" anchor for players still working towards their first win. The
// best-bv-per-second pair is the highest 3BV/s ever recorded on a *win*
// — independent of bestSeconds because a faster clock on a smaller
// board can yield a lower 3BV/s, and vice versa.
// The best-flag-accuracy pair is the highest flag-placement accuracy
// (correctFlags / flagsPlaced · 100, rounded to integer percent) ever
// recorded on a *loss* in this difficulty — surfaced as a Stats-dialog
// column to mirror the v1.32 per-loss "Correct flags: X / Y" line.
// Only updated on losses where the player placed at least one flag and
// the rounded accuracy strictly beats the prior record. 0 == no record
// yet; in production a value of 0 cannot persist (the gate requires a
// strictly-greater positive result), so the sentinel is always a clean
// "never recorded" signal — a lone wrong-flag loss leaves the field
// untouched.
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
    QDate bestStreakDate{};                   // invalid when no streak recorded yet
    std::uint32_t bestSafePercent{0};         // [0, 100]; 0 == no partial-clear record yet
    QDate bestSafePercentDate{};              // invalid when no partial-clear record yet
    double bestBvPerSecond{0.0};              // 0 == no 3BV/s record yet
    QDate bestBvPerSecondDate{};              // invalid when no 3BV/s record yet
    std::uint32_t bestFlagAccuracyPercent{0}; // [0, 100]; 0 == no flag-accuracy record yet
    QDate bestFlagAccuracyDate{};             // invalid when no flag-accuracy record yet
    // Lifetime sum of every counted winning duration in seconds — divisor for
    // the win-dialog "Average: %1" line. Only winning runs with seconds > 0
    // accumulate (mirrors the bestSeconds gate); replays and custom games are
    // excluded by `MainWindow::onGameWon` not calling `recordWin` for those.
    // 0.0 == no counted winning duration yet (first-load sentinel for legacy
    // pre-1.36 plists with no `total_seconds_won` key).
    double totalSecondsWon{0.0};
    // Calendar date (ISO 8601) on which the player's *most recent* counted win
    // for this difficulty completed. Distinct from `bestDate` (date of the
    // best-time run, which may be much older): `lastWinDate` is overwritten on
    // every counted `recordWin`, regardless of whether the run set a new best.
    // Drives the loss-dialog "Last win: %1" line, gated on `isValid()`.
    // Invalid (default-constructed) when the player has never won this
    // difficulty in 1.37+; pre-1.37 plists with `won > 0` but no
    // `last_win_date` key load as invalid by design — clean-slate seeding
    // surfaces the line on the *next* win, not retroactively.
    QDate lastWinDate{};
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
    // True iff `bvPerSecond` strictly beat the prior `bestBvPerSecond` for
    // this difficulty (including the very first win with `bvPerSecond > 0`,
    // when the field transitions 0.0 → some positive value). Drives the
    // win-dialog `⚡ New best 3BV/s!` flair.
    bool newBestBvPerSecond{false};

    // Total wins on this difficulty after this call (post-increment). The
    // win-dialog gates the "Average: %1" line on `winsAfter >= 3` — fewer
    // than three wins reduces to "average is the best time" (n=1) or "single
    // data point of variation" (n=2), neither informative.
    std::uint32_t winsAfter{0};
    // Mean of every counted winning duration on this difficulty after this
    // call: `totalSecondsWon / won`. 0.0 when `winsAfter == 0` or when no
    // counted winning duration exists yet (a sub-tick `seconds == 0.0` win
    // never accumulates, matching the bestSeconds sentinel). Pre-computed by
    // `Stats::recordWin` so callers don't re-load the record.
    double averageSecondsAfter{0.0};

    // Lets `QVERIFY(recordWin(...))` and `if (recordWin(...))` keep their
    // pre-WinOutcome bool semantics (true == new best-time set). Explicit so
    // accidental assignment to `bool` still requires `.newRecord`.
    explicit operator bool() const noexcept { return newRecord; }
};

// Outcome of a recordLoss call. `newBestSafePercent` is true iff the loss's
// `safePercent` strictly beat the prior `bestSafePercent` for this difficulty
// (including the very first loss with `safePercent > 0`, when the field
// transitions 0 → some positive value). Mirrors the WinOutcome.newRecord
// convention so `MainWindow` can surface a "🎯 New best %!" flair on the loss
// dialog parallel to the win-side "🏆 New record!".
struct LossOutcome
{
    bool newBestSafePercent{false};
    // True iff `flagAccuracyPercent` strictly beat the prior
    // `bestFlagAccuracyPercent` for this difficulty (including the very first
    // loss with `flagAccuracyPercent > 0`, when the field transitions 0 →
    // some positive value). Drives a future loss-dialog flair if/when one
    // ships; for now it's surfaced through telemetry alongside the existing
    // `new_best_safe_percent` tag so flag-accuracy distributions can be
    // analysed.
    bool newBestFlagAccuracyPercent{false};
    // Value of `currentStreak` immediately before this `recordLoss` call zeroed
    // it. Used by the loss dialog to surface a "💔 Streak ended at %1" line
    // when the broken streak was at least 2 (mirrors the win-side
    // `🔥 Streak: %1` gate at currentStreak >= 2). 0 == no streak was running
    // at loss time (first-ever loss, or previous game was already a loss).
    // Replays / custom games do not call `recordLoss`, so a default-constructed
    // LossOutcome carries 0 and the loss dialog hides the line — by design,
    // since those losses don't actually break the standard-difficulty streak.
    std::uint32_t priorStreak{0};

    // Mirror of WinOutcome::operator bool — explicit so accidental assignment
    // to `bool` still requires `.newBestSafePercent`. Lets future call sites
    // do `if (Stats::recordLoss(...))` if they want. Bool conversion
    // intentionally tracks ONLY `newBestSafePercent` so existing callers
    // (and the v1.29.0 🎯 flair gate) keep their pre-1.33 semantics — a
    // freshly-set flag-accuracy record on a loss that didn't also set a
    // safe-percent record returns false.
    explicit operator bool() const noexcept { return newBestSafePercent; }
};

[[nodiscard]] Record load(const QString &difficultyName);
void save(const QString &difficultyName, const Record &record);
void reset(const QString &difficultyName);
void resetAll();

// Convenience: increment Played (never mutates bestSeconds) and zero
// currentStreak. Used on loss. `safePercent` is the percentage of the board
// the player cleared at the moment of the loss (0..100); when strictly
// greater than the current `bestSafePercent`, it overwrites that field and
// stamps `bestSafePercentDate` with `onDate`. Defaults to 0 so existing
// callers (and tests) are source-compatible — a 0 percent will never set or
// touch the partial-clear best, mirroring the recordWin sentinel for 0
// seconds. `flagAccuracyPercent` is the rounded percentage of the user's
// placed flags that landed on actual mines at the moment of explosion
// (0..100); same semantics as `safePercent` — strictly greater than the
// current `bestFlagAccuracyPercent` overwrites the field and stamps
// `bestFlagAccuracyDate` with `onDate`; defaults to 0 so the 12 pre-1.33
// recordLoss test sites stay source-compatible. Returns LossOutcome so
// callers can surface "New best %!" flair on the end-of-game dialog;
// ignoring the return value is harmless.
LossOutcome recordLoss(const QString &difficultyName, int safePercent = 0, int flagAccuracyPercent = 0, const QDate &onDate = QDate::currentDate());

// Convenience: increment Played + Won, update bestSeconds if the run was
// faster (or no prior record), increment currentStreak, and roll bestStreak
// forward when current crosses it. Optionally update `bestBvPerSecond` if
// `bvPerSecond` strictly beats the prior record (mirrors the bestSeconds
// gate, but on the canonical efficiency axis instead of pure clock time).
// Returns the full outcome so the caller can surface "New record!" /
// "Best streak!" / "New best 3BV/s!" flair on the win dialog. `onDate` lets
// callers (primarily tests) inject the date stamped onto every new best
// touched by this call; in production it defaults to today. `bvPerSecond`
// defaults to 0.0 so existing callers (and 17 test sites) are
// source-compatible — a 0.0 will never set or touch the 3BV/s best,
// mirroring the bestSeconds 0.0-sentinel.
WinOutcome recordWin(const QString &difficultyName, double seconds, const QDate &onDate = QDate::currentDate(), double bvPerSecond = 0.0);

// Update only the no-flag best for a difficulty if the run was faster (or
// no prior no-flag record). Does NOT touch played/won/bestSeconds — call
// `recordWin` first for those, then call this when the run was completed
// without placing any flag. Returns true iff a new no-flag record was set.
bool recordNoflagBest(const QString &difficultyName, double seconds, const QDate &onDate = QDate::currentDate());
} // namespace Stats

#endif // STATS_H
