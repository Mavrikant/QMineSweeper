#ifndef STATS_H
#define STATS_H

#include <QString>

#include <cstdint>

// Per-difficulty lifetime statistics, persisted in QSettings under the
// stats/<DifficultyName>/{played,won,best_seconds} tree. Best-time is
// stored as seconds (double); 0 means "no win recorded yet".
namespace Stats
{
struct Record
{
    std::uint32_t played{0};
    std::uint32_t won{0};
    double bestSeconds{0.0}; // 0 == no record yet
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
bool recordWin(const QString &difficultyName, double seconds);
} // namespace Stats

#endif // STATS_H
