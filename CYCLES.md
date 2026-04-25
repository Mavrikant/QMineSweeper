# Autonomous cycles log

## 2026-04-25 — Cycle 29 — v1.32.0 (autonomous)

- **Chosen problem:** The loss dialog already shows total flags
  placed (v1.24.0) but doesn't tell the user how many of them were
  on actual mines. Did the player flag with discipline, or were
  they guessing? The total-flags line alone can't say. A
  `Correct flags: X / Y` companion line — same `flagsPlaced > 0`
  gate, same `correct / total` shape as `Partial 3BV: X / Y` —
  closes the obvious accuracy gap on the recent v1.21–v1.27
  loss-dialog detail thread.
- **Evidence:** v1.24.0 introduced `Flags placed: %1`. The
  `MineField::flagsPlaced()` doc comment explicitly notes that on a
  loss it gives "the user's actual flag count at the moment of
  explosion" — pairing that with correctness is the natural next
  step. No existing accuracy metric on the dialog. The walk over
  ≤ 480 cells is the same pattern `questionMarksPlaced()` already
  uses.
- **Shipped:**
  - Branch: `feat/v1.32.0-correct-flags-line` (squash-merged + deleted)
  - PR: [#50](https://github.com/Mavrikant/QMineSweeper/pull/50)
    (squash-merged as `70b786b`)
  - Release: https://github.com/Mavrikant/QMineSweeper/releases/tag/v1.32.0
  - Release workflow `24937956641` succeeded; all 4 platform assets
    + `SHA256SUMS.txt` published (Linux AppImage 35.9 MB / tar.gz
    35.6 MB, macOS .dmg 23.2 MB, Windows .zip 44.1 MB). Hand-written
    user-facing release notes installed via `gh release edit` —
    explains the line's form, the no-flag-boom gate, and the macOS
    quarantine clear.
- **Code surface:** ~30 LOC of production diff:
  - `minefield.h`: 11 LOC of doc-comment + 1 LOC method declaration
    for `correctFlagsPlaced()`.
  - `minefield.cpp`: 14 LOC implementation walking live cells for
    `isFlagged() && isMined()`. Mirrors `questionMarksPlaced()`
    exactly.
  - `mainwindow.h`: 1 LOC signature change to thread
    `lossCorrectFlags` through `showEndDialog`.
  - `mainwindow.cpp`: 4 LOC — fetch `correctFlags` in
    `onGameLost`, emit on telemetry as `correct_flags`, pass to
    `showEndDialog`, render the new line in the loss branch with
    `tr("Correct flags: %1 / %2")`.
  - `tests/tst_minefield.cpp`: 7 new test methods + 7 declarations
    (~80 LOC).
  - `apply_translations.py`: 9 LOC (1 new key × 9 locales).
  - `CMakeLists.txt`: 1 LOC (version bump).
  - Total real code+test diff: ~120 LOC; well under the 400-LOC
    cycle cap. (689/505 line-count is `lupdate`-driven `.ts` line
    renumbering across 9 locales.)
- **Tests added (7 new in `tst_minefield.cpp`):**
  - `testCorrectFlagsPlacedZeroBeforeAnyFlag` — empty initial
    state on a 3×3 board with two mines.
  - `testCorrectFlagsPlacedCountsOnlyFlagsOnMines` — mixed (1 of
    2): one flag on a mine, one flag on a safe cell. Catches a
    naive `m_flagCount`-mirror implementation.
  - `testCorrectFlagsPlacedAllOnMines` — boundary (2/2). The
    `correct ≤ total` invariant must allow equality.
  - `testCorrectFlagsPlacedNoneOnMines` — boundary (0/2). Player
    flagged only safe cells.
  - `testCorrectFlagsPlacedQuestionDoesNotCount` — regression
    guard: a `?` on a mined cell still has `m_isMined == true`,
    but `isFlagged()` is false. Catches any future change that
    conflates marker states.
  - `testCorrectFlagsPlacedPreservedOnLoss` — `revealAllMines()`
    must not touch flag state, mirroring the existing
    `testFlagsPlacedPreservedOnLoss` and
    `testQuestionMarksPlacedPreservedOnLoss`.
  - `testCorrectFlagsPlacedResetByNewGame` — `newGame()` clears
    prior cells; the next round starts at 0.
- **Translation cost:** 1 new hand-translated string × 9 non-en
  locales. 103/103 finished per locale, 0 unfinished — 50/50
  coverage preserved.
  - TR `"Doğru bayrak: %1 / %2"`
  - ES `"Banderas correctas: %1 / %2"`
  - FR `"Drapeaux corrects : %1 / %2"`
  - DE `"Korrekte Flaggen: %1 / %2"`
  - RU `"Верных флагов: %1 / %2"`
  - PT `"Bandeiras corretas: %1 / %2"`
  - ZH `"正确旗子：%1 / %2"`
  - HI `"सही झंडे: %1 / %2"`
  - AR `"الأعلام الصحيحة: %1 / %2"`
- **Assumptions made:**
  - **Separate line, not parenthetical** (`Flags placed: 8
    (correct: 6)`) — keeps the existing `Flags placed: %1`
    translation key stable across all 9 locales, avoids forcing
    re-translation. Two clean lines also read more naturally than
    a parenthetical and match the `Partial 3BV: X / Y` precedent.
  - **`correct / total` form, not percentage** — preserves
    absolute counts. With 1–2 flags placed, "50 %" reads
    awkwardly (1/2 vs 4/8 are different stories). Also avoids
    introducing a fourth percent sign on a dialog that already has
    `You cleared %1% of the board.`.
  - **Same `flagsPlaced > 0` gate as the Flags placed line** — a
    no-flag boom doesn't render either line. Always rendered
    when the companion is rendered: there's no scenario where
    Flags placed shows but Correct flags does not.
  - **Walk runs only at end-of-game** — same O(rows × cols) ≤ 480
    pattern as `questionMarksPlaced()`. No counter, no transition
    signal subscription. Flags can be placed before mines are
    seeded (mine placement is deferred to the first click), so a
    counter would have to re-tally on `placeMines` anyway.
  - **Telemetry tag added** — `game.lost` event carries
    `correct_flags` alongside `flags`/`qmarks`. Useful for future
    analysis of flag-accuracy distributions across difficulties.
    Same anonymous-tag profile as the existing per-event tags;
    no new PII.
- **Skipped:**
  - *Win-dialog "Correct flags" line.* The win path's
    `flagAllMines()` auto-flags every mine after the state flips
    to Won, so flags placed and correct flags would always be
    equal to mineCount. Useless on the win side; explicitly
    excluded by the `flagsPlaced > 0` placement on the loss-side
    `if` block (the win dialog does not even reach this code path
    because `won == true`).
  - *Stats dialog "Best flag accuracy" column.* Lifetime
    persistence would need a new `Record::bestFlagAccuracyPercent`
    + date pair. Defer to a future cycle — same shape as the
    v1.27 → v1.28 partial-3BV → hall-of-fame thread.
  - *Loss-dialog "Time since last win" line.* Re-parked from
    cycle 28's next-candidates. Needs a `last_win_date` field
    (separate from `bestDate`). ~30 LOC. Defer.
  - *Win-dialog "Average time" line.* Re-parked from cycle 28.
    Needs persistence of total seconds. ~40 LOC. Defer.
- **Risks logged:** none. Pure additive change. No schema
  migration. No new persistence. New telemetry tag is a string
  count — same anonymous-tag profile as the existing tags.
- **Self-review (adversarial pass):**
  - *What breaks in production?* The loss dialog gets one extra
    line when the player placed any flags. The `tr()` lookup
    falls back to source on any locale where the key is missing
    — verified `0` unfinished outside `en` so this won't trigger.
    A `tr()` source-string mismatch (typo) would print the source
    string instead of the translation; pinned by all 9 hand
    translations using the exact key.
  - *Backwards compat?* No API change to `Stats::Record`; no
    schema migration; no new QSettings keys. The
    `MineField::correctFlagsPlaced()` method is new but
    appended; no existing methods or signatures changed.
    `MainWindow::showEndDialog` got one extra trailing parameter
    — only two call sites, both updated atomically. A partial
    update would be caught at compile time.
  - *Error paths?* `correctFlagsPlaced()` is total: it walks the
    live cells and returns a non-negative int. Cannot throw,
    cannot return a sentinel. Empty board (no cells)
    safely returns 0.
  - *Secrets / PII?* New telemetry tag is a count (integer).
    No new persistence, no network calls beyond the existing
    Sentry pipeline. Strictly read-only on the field.
  - *Performance?* End-of-game-only walk over ≤ 480 cells
    (Expert). Sub-microsecond. Same pattern as
    `questionMarksPlaced()` which already runs once per loss.
  - *Concurrency?* Single-threaded. No new shared state.
  - *Q-mark regression?* Explicitly tested:
    `testCorrectFlagsPlacedQuestionDoesNotCount` confirms a `?`
    on a mined cell does not inflate the counter, because
    `isFlagged()` is false for `Question` markers.
- **Post-release watch (T+~3min):** Sentry
  `karaman/qminesweeper` — `search_issues` for unresolved issues
  in release `qminesweeper@1.32.0` in the last hour returned
  **zero results**. Expected — telemetry is opt-in and the assets
  just published with zero downloads. Watch closed.
- **Next candidates:**
  - **Stats-dialog "Best flag accuracy" column** — lifetime
    hall-of-fame for the v1.32 metric, same v1.27 → v1.28 pattern
    of per-loss readout → lifetime persistence. Needs
    `Record::bestFlagAccuracyPercent` + date pair on a non-replay,
    non-custom loss with `flagsPlaced > 0`. ~50 LOC.
  - **Loss-dialog "Time since last win" line** when the player has
    won this difficulty before. Re-parked from cycle 28. Needs
    a new `last_win_date` field. ~30 LOC + 1 translatable string
    × 9 locales.
  - **Win-dialog "Average time" line** showing the player's
    average winning time after >= 3 wins. Re-parked from cycle 28.
    Needs persistence of total seconds. ~40 LOC.
  - **Stats-dialog row totals** below the 3 difficulty rows —
    grand-total played / won / win % across all difficulties. Pure
    presentation aggregation; no new persistence. Re-parked from
    cycle 28.

## 2026-04-25 — Cycle 28 — v1.31.0 (autonomous)

- **Chosen problem:** The v1.30 cycle log explicitly tagged a Stats-
  dialog "Best 3BV/s" column as "the highest-value next-cycle filler
  … reads the just-shipped `bestBvPerSecond` + `bestBvPerSecondDate`
  fields, renders as a new 7th column on the Stats table." v1.30
  surfaced the lifetime best 3BV/s only as a one-shot
  `⚡ New best 3BV/s!` flair on the win dialog the moment the user
  set a new personal best — once the dialog closed, the value
  disappeared. The lifetime hall-of-fame counterpart was the
  obvious mirror feature.
- **Evidence:** Stats table had 6 columns (Difficulty / Played / Won
  / Best time / Best (no flag) / Streak); the persistence layer
  shipped in v1.30 (`Record::bestBvPerSecond` + `bestBvPerSecondDate`)
  was wired into `recordWin` and surfaced on the win dialog, but
  never read by `MainWindow::showStatsDialog`. Pure presentation
  gap.
- **Shipped:**
  - Branch: `feat/stats-best-bv-per-second-column` (squash-merged + deleted)
  - PR: [#49](https://github.com/Mavrikant/QMineSweeper/pull/49)
    (squash-merged as `07e239c`)
  - Release: https://github.com/Mavrikant/QMineSweeper/releases/tag/v1.31.0
  - Release workflow `24936839129` succeeded; all 5 assets +
    `SHA256SUMS.txt` published (Linux AppImage 35.9 MB / tar.gz
    35.6 MB, macOS .dmg 23.2 MB, Windows .zip 44.1 MB). Hand-written
    user-facing release notes installed via `gh release edit` —
    explains the format, the 3BV/s metric's meaning, the v1.30 flair
    relationship, the replay/custom exclusion, and platform-specific
    install notes (macOS quarantine clear).
- **Code surface:** ~10 LOC of production diff:
  - `mainwindow.cpp`: 3 LOC (column count 6→7, header label list
    gains `tr("Best 3BV/s")`, new `setItem(i, 6, …)` call) + 1 LOC
    new include for the helper.
  - `bv_per_second_format.h`: NEW. ~20 LOC pure inline helper
    `formatBvPerSecondCell(double, const QDate&)`. Analogous to
    `time_format.h`. Returns `"—"` for `bvPerSecond <= 0`,
    `"X.YZ"` for positive without date, `"X.YZ  (LL.LL.YYYY)"`
    with locale-formatted short date inlined.
  - `apply_translations.py`: 9 LOC (1 new key × 9 locales).
  - `tests/CMakeLists.txt`: 1 LOC (register new test).
  - `tests/tst_bv_per_second_format.cpp`: NEW. ~110 LOC, 11 cases.
  - `CMakeLists.txt`: 1 LOC (version bump).
  - Total real code+test diff: ~150 LOC; well under the 400-LOC
    cycle cap. (1081/793 line-count is `lupdate`-driven `.ts` line
    renumbering across 9 locales.)
- **Tests added (11 new in `tst_bv_per_second_format.cpp`):**
  - `testZeroRendersAsEmDash` — sentinel for no-record-yet.
  - `testNegativeRendersAsEmDash` — defence in depth against a
    future arithmetic-bug call site passing negative.
  - `testNanRendersAsEmDash` — IEEE-754 NaN comparisons are false,
    so the `!(x > 0.0)` guard catches NaN cleanly.
  - `testPositiveInfinityRendersAsEmDash` — pins current
    behaviour (printf renders `"inf"`, non-empty string asserted).
    Documented but not masked — callers should never pass +inf.
  - `testPositiveWithoutDateRendersTwoDecimals` — three values
    (`1.234→"1.23"`, `0.5→"0.50"`, `12.0→"12.00"`).
  - `testPositiveWithDateAppendsLocaleShortDate` — locale-pinned
    via `QLocale::setDefault` in `initTestCase` so the date format
    doesn't depend on the host running the suite.
  - `testTrailingZeroPreserved` — `%.2f` always emits 2 decimals
    (`2.10→"2.10"`, `2.00→"2.00"`); important so the column has a
    monospaced visual width across rows.
  - `testRoundsHalfAwayFromZero` — `1.236→"1.24"`, `1.234→"1.23"`.
  - `testVerySmallPositiveStillRenders` — sub-1 3BV/s on a tiny
    Beginner board (`0.005→"0.01"`).
  - `testInvalidDateFallsBackToValueOnly` — empty-ISO-string-derived
    invalid date drops parenthesised suffix (the persistence-layer
    no-record-yet load path).
  - **Adversarially verified** by mutating the em-dash sentinel
    `"—"` → `"XX"` (3 tests fail) and `%.2f` → `%.1f` (6 tests
    fail). The tests catch the bugs they're supposed to.
- **Translation cost:** 1 new hand-translated string × 9 non-en
  locales. 102/102 finished per locale, 0 unfinished — 50/50
  coverage preserved.
  - TR `"En iyi 3BV/sn"`
  - ES `"Mejor 3BV/s"`
  - FR `"Record 3BV/s"` (matches `Record (sans drapeau)`)
  - DE `"Bestwert 3BV/s"` (matches v1.30 flair `Neuer Bestwert 3BV/s!`;
    `Bestzeit` would be specifically best-*time* and mislead on a
    rate)
  - RU `"Лучшее 3BV/с"` (Cyrillic с for "сек.")
  - PT `"Recorde 3BV/s"`
  - ZH `"最佳 3BV/秒"`
  - HI `"सर्वश्रेष्ठ 3BV/से"` (Devanagari से for "सेकंड")
  - AR `"أفضل 3BV/ث"` (Arabic ث for "ثانية")
- **Assumptions made:**
  - **Two-decimal `%.2f`** matches the live win-dialog
    `"3BV: %1 · 3BV/s: %2"` line and the v1.30
    `⚡ New best 3BV/s!` flair, so the persisted record reads
    identically to the celebration. Pinned by
    `testTrailingZeroPreserved` and `testRoundsHalfAwayFromZero`.
  - **Free helper, not a private lambda** — past stats-dialog
    cycles (v1.28, v1.29, v1.30) tested only the persistence layer
    because the dialog lives behind `QDialog::exec()`. Extracting
    `formatBvPerSecondCell` into `bv_per_second_format.h` lets us
    unit-test it directly without standing up `tst_mainwindow`.
  - **Em-dash sentinel** for no-record-yet — same glyph the
    Best-time / Best-(no flag) cells already use; visual
    consistency across the table.
  - **Locale-formatted short date** in parentheses, two-space
    separator — mirrors Best-time / Best-(no flag) / Streak cells.
    No new column header per stat, no extra translatable string
    for the date format.
  - **No schema migration** — pure presentation read of v1.30
    fields. Pre-1.30 records (no `bestBvPerSecond` field saved)
    load as 0.0 / invalid date and render as em-dash. Pinned by
    v1.30's `testLegacyRecordWithoutBestBvPerSecondLoadsAsZero`.
  - **Replay/custom exclusion** is inherited automatically —
    `recordWin` is only called from `MainWindow::onGameWon`'s
    `!excludedFromStats` branch. Memorised-board runs never touch
    the field, so the new cell can never display a non-honest
    record.
- **Skipped:**
  - *Loss-dialog "Time since last win" line.* Needs a new
    `last_win_date` field (separate from `bestDate`, which is the
    date of the best-time win, not the most recent). Defer to a
    future cycle.
  - *Win-dialog "Average time" line.* Needs persistence of total
    seconds (or just total seconds, since `won` is already
    tracked). ~40 LOC. Defer.
  - *About-dialog mention of the new column.* Kept the about body
    byte-identical to avoid touching 10 hand translations.
- **Risks logged:** none. Pure presentation read of an existing
  field. No schema change. No new persistence. No new telemetry.
- **Self-review (adversarial pass):**
  - *What breaks in production?* Column count went 6 → 7; header
    labels list went 6 → 7 entries. Both updated atomically — a
    partial update would be caught at compile time, not runtime.
  - *Backwards compat?* No API change to `Stats::Record`; no
    schema migration; pre-1.30 records render as em-dash.
  - *Error paths?* `formatBvPerSecondCell` is total: every
    double / QDate combination produces a non-empty string. NaN
    and negative are masked to em-dash; +inf is documented as
    `"inf"` (callers should never pass it but if they do, the
    user sees `"inf"`, not a crash).
  - *Secrets / PII?* No new telemetry, no new persistence, no
    network calls. Strictly read-only.
  - *Performance?* Three new `QString` ops per Stats-dialog open
    × 3 difficulty rows. Sub-microsecond, dialog-open path only.
  - *Concurrency?* Single-threaded. No new shared state.
- **Post-release watch (T+~3min):** Sentry
  `karaman/qminesweeper` — `search_issues` for unresolved issues
  in release `qminesweeper@1.31.0` in the last hour returned
  **zero results**. Expected — telemetry is opt-in and the assets
  just published with zero downloads. Watch closed.
- **Next candidates:**
  - **Loss-dialog "Time since last win" line** when the player has
    won this difficulty before. Would need a new `last_win_date`
    field (separate from `bestDate`). ~30 LOC of production +
    persistence diff + 1 translatable string × 9 locales. A
    psychological nudge for players on a long losing streak.
  - **Win-dialog "Average time" line** showing the player's average
    winning time on this difficulty after >= 3 wins. Needs
    persistence of total seconds + count divisor. ~40 LOC.
  - **Stats-dialog row totals** below the 3 difficulty rows —
    grand-total played / won / win% across all difficulties. Pure
    presentation aggregation; no new persistence.

## 2026-04-25 — Cycle 27 — v1.30.0 (autonomous)

- **Chosen problem:** The "Win-dialog ⚡ New best 3BV/s flair" was
  the next-candidate item explicitly re-parked across cycles 24, 25,
  and 26. Three deferrals is enough — the persistence-layer cost is
  fixed (two new keys, two new fields) and small enough to fit a
  single cycle. 3BV/s is the canonical Minesweeper efficiency metric
  (already surfaced on every win dialog since v1.14.0 as the "3BV: %1
  · 3BV/s: %2" line); a new personal best at 3BV/s is the speedrun
  community's primary progress signal — much more meaningful than a
  faster clock time on a lower-3BV (i.e. easier) board.
- **Evidence:** `Stats::recordWin` returned a `WinOutcome` with
  `newRecord` (best clock) / `currentStreak` / `newBestStreak`, but
  no per-difficulty 3BV/s lifetime tracking. The win dialog had three
  flair slots filled (`🏆`, `🌟`/`🔥`, `🏃`) but no flair on the
  efficiency axis even though the inline "3BV: X · 3BV/s: Y" line
  has been there since v1.14.0.
- **Shipped:**
  - Branch: `feat/win-best-bv-per-second-flair` (squash-merged + deleted)
  - PR: [#48](https://github.com/Mavrikant/QMineSweeper/pull/48)
    (squash-merged as `fc2ddc7`)
  - Release: https://github.com/Mavrikant/QMineSweeper/releases/tag/v1.30.0
  - Release workflow `24935744804` succeeded; all 5 assets +
    `SHA256SUMS.txt` published (Linux AppImage 35.9 MB / tar.gz
    35.6 MB, macOS .dmg 23.2 MB, Windows .zip 44.1 MB). Hand-written
    user-facing release notes installed via `gh release edit` —
    explains the strict-greater-than gate, the standard-difficulties-
    only gate, the replay/custom exclusion, the independence from
    the existing 🏆 best-time flair, and the L→R render order on a
    multi-flair win.
- **Code surface:** ~100 LOC of production diff:
  - `stats.h`: 32 LOC (two new `Record` fields, new `WinOutcome`
    field, new `recordWin` trailing default-arg parameter, comment
    block update for the per-difficulty QSettings tree).
  - `stats.cpp`: 25 LOC (load/save/reset paths for two new keys,
    `recordWin` body gains a strict-greater-than gate that updates
    `bestBvPerSecond` + date and sets the new `WinOutcome` flag).
  - `mainwindow.{h,cpp}`: 35 LOC (`onGameWon` hoists `bv` + `bvRate`
    so they can be threaded into `recordWin`; `showEndDialog` gains
    a 16th positional parameter `winNewBestBvPerSecond`; new flair
    prepended last so it ends up leftmost; new `game.won` telemetry
    tag `new_best_bv_per_second`).
  - `apply_translations.py`: 9 LOC (1 new key × 9 locales).
  - +225 LOC of new tests in `tst_stats.cpp`. Real production+test
    diff well under the 400-LOC cycle cap (the 939/510 line-count is
    `lupdate`-driven `.ts` line renumbering across 9 locales).
- **Tests added (15 new in `tst_stats.cpp`):**
  - `testBestBvPerSecondDefaultsZero` — virgin record sanity
    baseline.
  - `testRecordWinDefaultArgsKeepsBestBvPerSecondZero` — source-compat
    default-arg path: 17 pre-1.30 test sites still work and never
    touch the new field.
  - `testRecordWinWithBvRateSetsOnFirstCall` — first record sets
    both value and date.
  - `testRecordWinWithHigherBvRateBeats` — strict beat fires.
  - `testRecordWinWithLowerBvRateKeepsOriginal` — worse rate doesn't.
  - `testRecordWinWithEqualBvRateKeepsOriginalDate` — strict
    greater-than: ties keep the original date (mirrors best-streak
    convention).
  - `testRecordWinWithZeroBvRateNoOp` — sub-tick win sentinel
    short-circuits the update path.
  - `testRecordWinWithNegativeBvRateNoOp` — defence in depth against
    a future arithmetic-bug call site passing negative.
  - `testRecordLossDoesNotTouchBestBvPerSecond` — load-bearing
    invariant: a loss must not zero/overwrite the field.
  - `testBestBvPerSecondIsPerDifficulty` — Beginner record doesn't
    leak into Intermediate/Expert.
  - `testResetWipesBestBvPerSecond` /
    `testResetAllWipesBestBvPerSecond` — both new keys cleared on
    reset paths.
  - `testLegacyRecordWithoutBestBvPerSecondLoadsAsZero` — pre-1.30
    QSettings tree (no new keys) loads as 0.0 / invalid date.
  - `testFasterClockMayNotBeatBvRateAndViceVersa` — independence
    pin for the two records (a 3-run scenario walking through both
    independence directions).
  - `testWinOutcomeBvRateIndependentOfNewRecord` — full
    `WinOutcome` field-matrix pin.
  - **Adversarially verified** by mutating
    `newBestBvPerSecond = true` to `= false` in `Stats::recordWin`
    — 9 of 15 new tests fail with the wrong value. The 6 negative-
    path tests correctly stay green — they verify the field is NOT
    set in those branches, which the mutation doesn't affect.
- **Translation cost:** 1 new hand-translated string × 9 non-en
  locales. 50/50 coverage preserved (all 9 .ts files
  `Generated 101 translation(s) (101 finished and 0 unfinished)`).
  - TR `"⚡ Yeni en iyi 3BV/sn!"`
  - ES `"⚡ ¡Nuevo mejor 3BV/s!"`
  - FR `"⚡ Nouveau meilleur 3BV/s !"` (French thin-space-before-!)
  - DE `"⚡ Neuer Bestwert 3BV/s!"`
  - RU `"⚡ Новый лучший 3BV/с!"` (Cyrillic с for "сек.")
  - PT `"⚡ Novo melhor 3BV/s!"`
  - ZH `"⚡ 新最佳 3BV/秒！"` (full-width !)
  - HI `"⚡ नया सर्वश्रेष्ठ 3BV/से!"` (Devanagari से for "सेकंड")
  - AR `"⚡ أفضل 3BV/ث جديد!"` (Arabic ث for "ثانية")
- **Assumptions made:**
  - **Strict greater-than gate** for the flair (ties don't fire).
    Pinned by `testRecordWinWithEqualBvRateKeepsOriginalDate`.
    Mirrors the persistence layer's date-pin convention from
    cycles 22 and 25.
  - **Replay / custom losses excluded** — naturally inherited from
    the existing v1.13.0 `!excludedFromStats` gate around
    `MainWindow::onGameWon`'s `Stats::recordWin` call. No new gate
    needed.
  - **Sub-tick win (bvRate==0) never flairs** — `recordWin`'s
    `bvPerSecond > 0.0` short-circuit prevents both persistence and
    the flair simultaneously. Pinned by
    `testRecordWinWithZeroBvRateNoOp`.
  - **Independent of best-time** — best-clock and best-3BV/s are
    independent records. Pinned by
    `testFasterClockMayNotBeatBvRateAndViceVersa` (3-run scenario:
    both axes set on run 1; clock-only on run 2; 3BV/s-only on
    run 3).
  - **Source-compat trailing-default change** on `recordWin`
    (`bvPerSecond = 0.0` trailing default). All existing call sites
    (1 production, 17 in tests) pass no value and remain unchanged.
  - **16th parameter on `showEndDialog`**, not a struct refactor.
    Adding the param matched the 14→15 pattern from v1.28.0 →
    v1.29.0; a struct refactor would be churn for no net benefit
    at this size.
  - **L→R render order** ⚡ → 🌟/🔥 → 🏆 → 🏆 → 🏃. The new flair
    prepends LAST in code so it ends up LEFTMOST visually — gives
    the new feature top billing on its release.
- **Skipped:**
  - *Stats-dialog "Best 3BV/s" column.* The Stats-side counterpart to
    this flair. Bundle into a near-future cycle to amortise the
    column-header translation cost (10 locales × 1 string).
    Re-parked as the natural one-cycle filler — same shape as the
    cycle 24 → 25 thread (per-run partial 3BV → lifetime hall-of-
    fame for the same metric).
  - *Loss-dialog "Time since last win" line.* Needs a new
    `last_win_date` field (separate from `bestDate`, which is the
    date of the best-time win, not the most recent). Defer to a
    future cycle.
  - *About-dialog mention of the new feature.* Kept the about body
    byte-identical to avoid touching 10 hand translations.
- **Risks logged:** none. Trailing-default-parameter change with
  source-compat for all 18 existing call sites; additive
  `WinOutcome` field; additive 16th `showEndDialog` parameter;
  additive QSettings keys with safe absent-key defaults; additive
  telemetry tag; no behavioural change on the loss path or any
  excluded win path.
- **Self-review (adversarial pass):**
  - *What breaks in production?* The 16th param on `showEndDialog`
    was added with both call sites (`onGameWon` passes
    `outcome.newBestBvPerSecond`, `onGameLost` passes `false`)
    updated atomically in the same commit; a partial update would
    be caught at compile time, not runtime.
  - *Backwards compat?* `Stats::recordWin` gains a trailing default
    parameter (source-compat); `WinOutcome` gains a trailing field
    (additive); existing readers of `newRecord` / `currentStreak` /
    `newBestStreak` are unaffected. New QSettings keys load as
    0.0/invalid for pre-1.30 records — pinned by
    `testLegacyRecordWithoutBestBvPerSecondLoadsAsZero`.
  - *Error paths?* The flair gate (`winNewBestBvPerSecond`) is
    purely additive in `showEndDialog`; if it's wrongly true, the
    user sees a redundant celebratory line — non-destructive. If
    wrongly false, they miss a celebration but nothing breaks. The
    persistence layer's `bvPerSecond <= 0.0` guard short-circuits
    sub-tick / negative cases.
  - *Secrets / PII?* New telemetry tag is a single boolean
    (`new_best_bv_per_second`) derived from public game state; not
    PII. No new local-filesystem writes beyond the additive
    QSettings keys.
  - *Performance?* Adds one double-comparison + one struct-field
    assignment on the win path — sub-microsecond, not in any hot
    path. Dialog render adds one conditional + one string prepend.
  - *Concurrency?* Single-threaded. No new shared state. The win
    flow is fully synchronous: `gameWon` signal → slot →
    `recordWin` → `showEndDialog`.
- **Post-release watch (T+~3min):** Sentry
  `karaman/qminesweeper` — `search_issues` for unresolved issues in
  release `qminesweeper@1.30.0` in the last hour returned **zero
  results**. Expected — telemetry is opt-in and the assets just
  published with zero downloads. Watch closed.
- **Next candidates:**
  - **Stats-dialog "Best 3BV/s" column** as the lifetime hall-of-
    fame counterpart to today's flair. Reads the just-shipped
    `bestBvPerSecond` + `bestBvPerSecondDate` fields, renders as a
    new 7th column on the Stats table. ~50 LOC + 1 translatable
    column header × 9 locales. Highest-value next-cycle filler.
  - **Loss-dialog "Time since last win" line** when the player has
    won this difficulty before. Would need a new `last_win_date`
    field (separate from `bestDate`). ~30 LOC of production +
    persistence diff + 1 translatable string × 9 locales. A
    psychological nudge for players on a long losing streak.
  - **Win-dialog "Average time" line** showing the player's average
    winning time on this difficulty after >= 3 wins. Needs
    persistence of total seconds + count divisor (or just total
    seconds, since `won` is already tracked). ~40 LOC.

## 2026-04-25 — Cycle 26 — v1.29.0 (autonomous)

- **Chosen problem:** Cycle 25 explicitly parked the loss-dialog
  `🎯 New best %!` flair as "the natural one-cycle filler — mirrors
  the win-side `🏆 New record!` pattern on the loss side." The
  persistence layer (`bestSafePercent` + `bestSafePercentDate` per
  difficulty, with replay/custom exclusion) shipped in v1.28.0; the
  user-visible flair on the loss dialog was the missing half. Until
  this release, a loss that pushed the per-difficulty hall-of-fame
  bar updated `QSettings` silently — the loss dialog gave no signal
  the run had set a new record. Players working towards their first
  win on a difficulty (especially Expert, where the first win can
  take weeks) had no in-the-moment feedback that progress was being
  made.
- **Evidence:** `Stats::recordLoss` was `void` and the persistence
  bump was invisible to `MainWindow::onGameLost`. The win path
  already had `WinOutcome.newRecord` driving the prepended
  `🏆 New record!` text on the win dialog (mainwindow.cpp:822-825);
  the loss path had no analogous mechanism.
- **Shipped:**
  - Branch: `feat/loss-dialog-best-percent-flair` (squash-merged + deleted)
  - PR: [#47](https://github.com/Mavrikant/QMineSweeper/pull/47)
    (squash-merged as `1813484`)
  - Release: https://github.com/Mavrikant/QMineSweeper/releases/tag/v1.29.0
  - Release workflow `24934585136` succeeded in 2m17s; all 5 assets
    + `SHA256SUMS.txt` published (Linux AppImage / tar.gz, macOS
    universal .dmg, Windows .zip). Hand-written user-facing release
    notes installed via `gh release edit` — explains the
    strict-improvement gate, the standard-difficulties-only gate,
    and the no-flair-on-first-click-boom rule.
- **Code surface:** ~70 LOC of production diff:
  - `stats.h`: new `LossOutcome { newBestSafePercent }` struct with
    explicit `operator bool()` (mirrors `WinOutcome`).
  - `stats.cpp`: `recordLoss` return type `void → LossOutcome`; sets
    the flag inside the existing strict-greater-than branch.
  - `mainwindow.{h,cpp}`: `showEndDialog` gains
    `bool lossNewBestSafePercent` (15th param); `onGameLost`
    captures `Stats::recordLoss(...)` outcome and threads it to the
    dialog; `game.lost` telemetry gains
    `new_best_safe_percent` tag.
  - +82 LOC of new tests in `tst_stats.cpp`. The 657/480 line-count
    is `lupdate`-driven `.ts` line renumbering.
- **Tests added (8 new in `tst_stats.cpp`):**
  - `testLossOutcomeDefaultArgsReturnsNoNewBest` — default-arg path
    cannot set the flag (safePercent==0 short-circuits).
  - `testLossOutcomeWithZeroPercentReturnsNoNewBest` — explicit 0
    (e.g. first-click boom) never flairs.
  - `testLossOutcomeFirstPositivePercentReturnsNewBest` — virgin
    record + first positive percent fires the flag (parallel to the
    first-win-is-also-a-record case).
  - `testLossOutcomeHigherPercentReturnsNewBest` — strict beat
    fires.
  - `testLossOutcomeEqualPercentReturnsNoNewBest` — strict
    greater-than: ties don't fire.
  - `testLossOutcomeLowerPercentReturnsNoNewBest` — worse loss
    doesn't fire.
  - `testLossOutcomeOverflowAtCapReturnsNoNewBest` — once the record
    has been clamped to 100, a subsequent overflow call is a tie,
    not a new best (no double flair).
  - `testLossOutcomeBoolConversion` — explicit `operator bool()`
    tracks `newBestSafePercent`.
  - Adversarially validated by inverting
    `newBestSafePercent = true` to `= false` in `Stats::recordLoss`
    — 3 of 8 tests fail (the positive-path tests:
    `testLossOutcomeFirstPositivePercentReturnsNewBest`,
    `testLossOutcomeHigherPercentReturnsNewBest`,
    `testLossOutcomeBoolConversion`). The 5 negative-path tests
    correctly stay green — they verify the field is NOT set in
    those branches, which the mutation doesn't affect.
- **Translation cost:** 1 new hand-translated string × 9 non-en
  locales. 50/50 coverage preserved (all 9 .ts files
  `Generated 100 translation(s) (100 finished and 0 unfinished)`).
  - TR `"🎯 Yeni en iyi %!"`
  - ES `"🎯 ¡Nuevo mejor %!"`
  - FR `"🎯 Nouveau meilleur % !"` (French thin-space-before-!)
  - DE `"🎯 Neuer Bestwert %!"`
  - RU `"🎯 Новый лучший %!"`
  - PT `"🎯 Novo melhor %!"`
  - ZH `"🎯 新最佳 %！"` (full-width !)
  - HI `"🎯 नया सर्वश्रेष्ठ %!"`
  - AR `"🎯 أفضل % جديد!"`
- **Assumptions made:**
  - **Strict greater-than gate** for the flair (ties don't fire).
    Pinned by `testLossOutcomeEqualPercentReturnsNoNewBest`. Matches
    the persistence layer's date-pin convention from v1.28.0.
  - **Replay / custom losses excluded** — naturally inherited from
    the existing v1.28.0 gate around `Stats::recordLoss`; the
    LossOutcome struct on a default-constructed `lossOutcome{}` has
    `newBestSafePercent == false`, so the flair is unreachable on
    the excluded paths without any new conditional in
    `MainWindow::onGameLost`.
  - **First-click boom (0%) never flairs** — `recordLoss`'s
    `safePercent > 0` short-circuit prevents both persistence and
    the flair simultaneously. Pinned by
    `testLossOutcomeWithZeroPercentReturnsNoNewBest`.
  - **Source-compat trailing-return change** on `recordLoss`
    (`void → LossOutcome`). All existing call sites (1 production,
    19 in tests) discard the value and remain unchanged — C++
    permits discarding non-void returns.
  - **15th parameter on showEndDialog**, not a struct refactor.
    Adding the param matched the existing 14-param signature shape
    (the function already threads many independent fields); a
    struct refactor would be churn for no net benefit at this size.
- **Skipped:**
  - *Lifetime "first-loss date" stamp.* Cosmetic, parked.
  - *Win-dialog "Personal best 3BV/s" flair.* Re-parked from cycle
    24 — needs new `Stats::recordBestBvPerSecond` accessor +
    persistence keys, larger surface than the cycle cap.
  - *Stats-dialog `🎯` icon prefix on the partial-clear cell.*
    Tempting but means a 2nd-cell glyph that the player only sees
    in the Stats dialog, not in the moment. The loss-dialog flair
    is the higher-impact placement.
- **Risks logged:** none. Trailing-return change with discardable
  result; additive 15th `showEndDialog` parameter; additive
  telemetry tag; no behavioural change on the win path or any
  excluded loss path.
- **Self-review (adversarial pass):**
  - *What breaks in production?* The 15th param on `showEndDialog`
    was added with both call sites (`onGameWon` passes `false`,
    `onGameLost` passes the outcome's flag) updated atomically in
    the same commit; a partial update would be caught at compile
    time, not runtime.
  - *Backwards compat?* `Stats::recordLoss` gains a non-void return
    type; existing `void`-discarding call sites compile unchanged.
    `LossOutcome` is value-copy-cheap (single bool); no allocation
    or RAII concern. Persistence layer unchanged from v1.28.0 — no
    QSettings schema bump.
  - *Error paths?* The flair gate (`lossNewBestSafePercent`) is
    purely additive in `showEndDialog`; if it's wrongly true, the
    user sees a redundant celebratory line — non-destructive. If
    wrongly false, they miss a celebration but nothing breaks. The
    persistence layer's behaviour is unchanged.
  - *Secrets / PII?* New telemetry tag is a single boolean
    (`new_best_safe_percent`) derived from public game state; not
    PII. No new local-filesystem writes (the QSettings write was
    already in v1.28.0).
  - *Performance?* Adds one bool assignment + one struct return on
    the loss path — sub-microsecond, not in any hot path. Dialog
    render adds one conditional + one string prepend.
  - *Concurrency?* Single-threaded. No new shared state. The loss
    flow is fully synchronous: `gameLost` signal → slot →
    `recordLoss` → `showEndDialog`.
- **Post-release watch (T+~3min):** Sentry
  `karaman/qminesweeper` — `search_issues` for unresolved issues in
  release `qminesweeper@1.29.0` in the last hour returned **zero
  results**. Expected — telemetry is opt-in and the assets just
  published with zero downloads. Watch closed.
- **Next candidates:**
  - **Win-dialog "Personal best 3BV/s" flair** when the run beats
    the per-difficulty best 3BV/s ever recorded. Parallel to the
    "🏆 New record!" flair (which gates on best-time), but for the
    canonical efficiency metric. Needs a new
    `Stats::recordBestBvPerSecond` accessor + lifetime persistence
    in `~/Library/Preferences/com.mavrikant.QMineSweeper.plist`.
    Re-parked from cycles 24+25.
  - **Stats-dialog "Best 3BV/s" column** as the Stats-side
    counterpart to the above flair. Bundle into the same cycle to
    amortise the persistence-layer cost.
  - **Loss-dialog "Time since last win" line** when the player has
    won this difficulty before. Reads `bestDate` (already
    persisted) and renders "X days since your last win." A
    psychological nudge for players on a long losing streak. No
    persistence churn, ~30 LOC + 1 translatable string × 9 locales.

## 2026-04-25 — Cycle 25 — v1.28.0 (autonomous)

- **Chosen problem:** Cycle 24 explicitly parked the Stats-dialog
  partial-clear hall-of-fame as "the highest-value remaining feature":
  for any difficulty a player has never won, the **Best time** column
  rendered a bare em-dash forever. New players (and Expert in general)
  can stare at three em-dashes for weeks without any indicator of
  progress. Cycle 24's partial-3BV loss-dialog line is per-run; this
  cycle is the *lifetime* counterpart on the Stats dialog itself.
- **Evidence:** `MainWindow::showStatsDialog` rendered
  `formatBest(rec.bestSeconds, rec.bestDate)` for every Best-time
  cell — when `bestSeconds <= 0.0` it emitted a bare `"—"`. There was
  no per-difficulty record of how close the player has come on a
  loss; `Stats::recordLoss(diffName)` only incremented `played` and
  reset `currentStreak`.
- **Shipped:**
  - Branch: `feat/stats-best-partial-percent` (squash-merged + deleted)
  - PR: [#46](https://github.com/Mavrikant/QMineSweeper/pull/46)
    (squash-merged as `318e451`)
  - Release: https://github.com/Mavrikant/QMineSweeper/releases/tag/v1.28.0
  - Release workflow `24933456501` green; all 5 assets +
    `SHA256SUMS.txt` published (Linux AppImage 35.9 MB / tar.gz
    35.6 MB, macOS .dmg 23.2 MB, Windows .zip 44.1 MB). Hand-written
    user-facing release notes installed via `gh release edit` —
    explains the "— (best 87%, 25.04.2026)" cell format and the
    once-you-win-it-supersedes rule.
- **Code surface:** ~140 LOC of production diff across `stats.{h,cpp}`
  (two new fields on Record, load/save/reset paths, `recordLoss` gains
  optional `safePercent`/`onDate` parameters, strict-greater-than
  semantics with a defence-in-depth clamp at 100) +
  `mainwindow.{h,cpp}` (hoist `safePercentCleared()` in `onGameLost`
  to thread it to `Stats::recordLoss`; new `formatBestTimeOrPartial`
  lambda in `showStatsDialog`). +170 LOC of new tests in
  `tst_stats.cpp`. The 763/474 line-count number is `lupdate`-driven
  `.ts` line renumbering. Real production+test diff well under the
  400-LOC cycle cap.
- **Tests added (14 new, all in `tst_stats.cpp`):**
  - `testBestSafePercentDefaultsZero` — virgin record sanity baseline.
  - `testRecordLossDefaultArgsKeepBestSafePercentZero` — source-compat
    default-arg path: legacy callers still work and never touch the
    new field.
  - `testRecordLossWithPercentSetsOnFirstCall` — first record sets
    both value and date.
  - `testRecordLossWithHigherPercentBeats` — higher overwrites both.
  - `testRecordLossWithLowerPercentKeepsOriginal` — lower keeps both.
  - `testRecordLossWithEqualPercentKeepsOriginalDate` — strict
    greater-than (ties keep the original date, mirroring best-streak).
  - `testRecordLossWithZeroPercentDoesNotMutate` — explicit 0 is a
    no-op (mirror of `recordWin`'s 0-second sentinel).
  - `testRecordLossWithFullClearPercentBoundary` — 100 boundary
    (defensible: a chord-click loss after the last open).
  - `testRecordLossWithOverflowPercentClampedTo100` — defence in depth
    against a future arithmetic-bug call site passing > 100.
  - `testRecordWinDoesNotTouchBestSafePercent` — load-bearing
    invariant: a win must not zero/overwrite the partial-clear field.
  - `testBestSafePercentIsPerDifficulty` — Beginner record doesn't
    leak into Intermediate/Expert.
  - `testResetWipesBestSafePercent`,
    `testResetAllWipesBestSafePercent` — both new keys cleared on
    reset paths.
  - `testLegacyRecordWithoutBestSafePercentLoadsAsZero` — pre-1.28
    QSettings tree (no new keys) loads as 0 / invalid date.
  - Adversarially verified by mutating
    `clamped > r.bestSafePercent` to `false` — 9 of 14 new tests
    fail with the wrong value. The 5 that pass trivially are
    zero-baseline tests that expect an unchanged zero record (that's
    exactly how to identify which tests are load-bearing for the
    value path).
- **Translation cost:** 1 new hand-translated string × 9 non-en
  locales. 50/50 coverage preserved (all 9 .ts files
  `Generated 99 translation(s) (99 finished and 0 unfinished)`).
  - TR `"— (en iyi %1%, %2)"`
  - ES `"— (mejor %1%, %2)"`
  - FR `"— (meilleur %1%, %2)"`
  - DE `"— (Beste %1%, %2)"`
  - RU `"— (лучший %1%, %2)"`
  - PT `"— (melhor %1%, %2)"`
  - ZH `"— (最高 %1%，%2)"` (full-width comma)
  - HI `"— (सर्वश्रेष्ठ %1%, %2)"`
  - AR `"— (الأفضل %1%، %2)"` (Arabic comma)
- **Assumptions made:**
  - **Show only when `won == 0`.** Once a win is recorded, the
    regular best-time format takes the cell back; surfacing both
    would clutter the cell with redundant info (the win clears 100
    %, by definition higher than any partial). The
    `bestSafePercent` value persists in QSettings as a silent
    record but is no longer surfaced.
  - **Strict greater-than semantics (no ties bump the date).**
    Mirrors the best-streak convention. Pinned by
    `testRecordLossWithEqualPercentKeepsOriginalDate`.
  - **Replays / custom games excluded.** Same rationale as wins: a
    memorised board would let the user inflate the lifetime record.
  - **One inline format string, not a new column.**
    `"— (best %1%, %2)"` inline in the existing Best-time column
    avoids a 7th column header × 10 locales. Pattern matches
    cycle 1's inline-date approach in the same lambda.
  - **uint storage, not float.** Game UI already rounds to integer
    percent on the loss dialog; a float would invite drift between
    the loss dialog (rounded int) and the Stats hall-of-fame
    (whichever precision was stored).
  - **Source-compat default-arg signature on `recordLoss`.** Adding
    `safePercent = 0` and `onDate = QDate::currentDate()` as
    trailing defaults keeps every existing test call site (18 in
    `tst_stats.cpp`) source-compat. The 0 default never touches the
    new field, mirroring the `recordWin` 0-seconds sentinel.
- **Skipped:**
  - *Per-difficulty Best % column.* Would add a 7th column header to
    translate × 10 locales for incremental info. Inline annotation
    in the existing Best-time column is cheaper.
  - *Loss-dialog `🎯 New best %!` flair on a fresh record.* Tempting
    and parallel to the win-side `🏆 New record!` /
    `🌟 New best streak!` pattern, but means another translatable
    string + a `LossOutcome` struct (returning
    `bool newPercentRecord`). Park as a one-cycle filler for the
    next cycle.
  - *Telemetry tag `new_best_safe_percent` on `game.lost`.* Same
    reasoning as the flair — if the flair gets shipped, the tag
    rides along with it.
  - *About-dialog mention of the new feature.* Kept the about body
    byte-identical to avoid touching 10 hand translations.
- **Risks logged:** none. Additive QSettings keys with safe absent-key
  defaults; additive trailing parameters on `Stats::recordLoss` with
  defaults; additive trailing fields on `Stats::Record`; no
  behavioural change on win or replay paths.
- **Self-review (adversarial pass):**
  - *What breaks in production?* The QSettings upgrade path: a
    player upgrading from 1.27.x has no
    `best_safe_percent_*` keys; the
    `testLegacyRecordWithoutBestSafePercentLoadsAsZero` test pins
    the load-as-zero invariant. After the first 1.28.0 loss with
    `> 0 %` cleared, the keys appear naturally.
  - *Backwards compat?* `Stats::recordLoss` gains two trailing
    default parameters; all existing call sites (1 in production,
    18 in tests) remain source-compat. `Stats::Record` gains two
    trailing fields; `load()` initialises them safely from absent
    QSettings keys. The `formatBest` lambda is untouched — the
    partial-clear branch lives in a sibling lambda that only fires
    when `bestSeconds <= 0.0`.
  - *Error paths?* The `safePercent > 100` branch clamps to 100
    (test: `testRecordLossWithOverflowPercentClampedTo100`).
    Negative values are filtered by the outer `safePercent > 0`
    guard. Date validity is checked in the renderer
    (`bestSafePercentDate.isValid()`) so a partial-but-undated
    record falls back to a bare em-dash rather than rendering
    `"— (best 87%, )"`.
  - *Secrets / PII?* No new telemetry fields. Persistence is local
    QSettings only. `safePercentCleared` is an integer in
    `[0, 100]` derived from public game state — not PII.
  - *Performance?* `recordLoss` adds one integer compare and one
    date assignment on the loss path only — sub-microsecond, not
    in any hot path. Stats dialog open is the only render path;
    one extra conditional per row.
  - *Concurrency?* Single-threaded. No new shared state.
- **Post-release watch (T+~5min):** Sentry
  `karaman/qminesweeper` — `search_issues` for unresolved issues in
  release `qminesweeper@1.28.0` in the last hour returned **zero
  results**. Expected — telemetry is opt-in and the assets just
  published with zero downloads. The signal worth watching for is
  *any* new group tagged with the 1.28.0 release; none observed.
  Watch closed.
- **Next candidates:**
  - **Loss-dialog `🎯 New best %!` flair when a loss sets a new
    `bestSafePercent`.** Now that the persistence layer is in place,
    the flair is the natural one-cycle filler — mirrors the
    win-side `🏆 New record!` pattern on the loss side.
    `Stats::recordLoss` would return a `LossOutcome { bool
    newPercentRecord }` analogous to `WinOutcome`. ~70-line
    production cost; 1 new translatable string × 9 locales.
  - **Win-dialog "Personal best 3BV/s" flair when the run beats the
    per-difficulty best 3BV/s ever recorded.** Parallel to the
    "🏆 New record!" flair (which gates on best-time), but for the
    canonical efficiency metric. Needs a new
    `Stats::recordBestBvPerSecond` accessor + lifetime persistence
    in `~/Library/Preferences/com.mavrikant.QMineSweeper.plist`.
    Re-parked from cycle 24.
  - **Lifetime "first-win date" stamp on Stats dialog.** When a
    player records their first-ever win on a difficulty, stamp the
    date (already happens via `bestDate`). Surface a "first won:"
    line on the Played column tooltip — no new persistence, just
    rendering. Cosmetic.

## 2026-04-25 — Cycle 24 — v1.27.0 (autonomous)

- **Chosen problem:** Cycle 23 explicitly parked partial 3BV on the
  loss dialog as the highest-impact next big feature: "Now that the
  static board 3BV (cycle 22) is shipped, the natural next step is
  the *partial* — `Partial 3BV: X / Y · 3BV/s: Z` so a speedrunner
  sees their per-second pace at the moment of explosion." The
  v1.25.0 line told the player how hard the board *was* but said
  nothing about how far *they* got through it.
- **Evidence:** `MainWindow::onGameLost` reads `boardValue()` from
  MineField but throws away the player's progress against it. No
  partial-bv accessor existed. The win path already renders
  `3BV: %1 · 3BV/s: %2` (cycle 14); the loss path had no equivalent.
- **Shipped:**
  - Branch: `feat/loss-dialog-partial-3bv` (squash-merged + deleted)
  - PR: [#45](https://github.com/Mavrikant/QMineSweeper/pull/45)
    (squash-merged as `eb07497`)
  - Release: https://github.com/Mavrikant/QMineSweeper/releases/tag/v1.27.0
  - Release workflow `24932359517` green; all 5 assets +
    `SHA256SUMS.txt` published (Linux AppImage 35.9 MB / tar.gz
    35.6 MB, macOS .dmg 23.2 MB, Windows .zip 44.1 MB, parallel
    to v1.26.0). Hand-written user-facing release notes installed
    via `gh release edit` — explains the three numbers
    (Partial / Total / 3BV/s) for new readers, names the
    Minesweeper-Arbiter "Cleared 3BV" convention, and notes that
    the line replaces (not adds to) v1.25's static board-3BV line.
- **Code surface:** ~150 LOC of production diff across
  `mainwindow.{h,cpp}` (read `partialBoardValue()` on loss + sub-tick
  guard for bvPerSecond, thread to `showEndDialog`, replace gated
  line, telemetry tags) + `minefield.{h,cpp}` (new
  `partialBoardValue()` accessor that re-walks compute3BV()'s
  region/isolated-cell partition counting only opened units) + 105
  LOC of new tests. The 765/507 line-count number is `lupdate`-
  driven `.ts` line renumbering (auto-regenerated). Production diff
  well under the 400-LOC cycle cap.
- **Tests added (8 new, all in `tst_minefield.cpp`):**
  - `testPartialBoardValueZeroBeforeMinesPlaced` — sanity baseline.
  - `testPartialBoardValueZeroBeforeAnyOpen` — placed-but-not-touched.
  - `testPartialBoardValueRegionCountedWhenAnyCellOpened` — opening
    partition: any cell of an opening counts the whole region as +1.
  - `testPartialBoardValueIsolatedNumberedCellsCountedIndividually`
    — isolated-cell partition: each opened cell is +1.
  - `testPartialBoardValueTwoSeparateOpenings` — multi-region
    accounting via the existing 3x7-with-mine-wall layout.
  - `testPartialBoardValueEqualsBoardValueOnWin` — anchor
    postcondition: partialBV == boardValue() at the win endpoint.
    Pins the canonical-3BV definition; would catch a regression to
    per-cell counting semantics.
  - `testPartialBoardValuePreservedOnLoss` — load-bearing for the
    new dialog line. Pins that revealAllMines does not zero the
    partial-bv value (any future change that resets opened-state on
    loss reveal would silently zero the dialog metric).
  - `testPartialBoardValueResetByNewGame` — newGame zeroes the
    count.
  - Adversarially verified by zeroing the function body and
    rebuilding — 6 of 8 tests fail with the wrong value (the 2
    zero-baseline tests pass trivially because they expect 0; that's
    how to identify which tests are load-bearing for the value
    path).
- **Translation cost:** Old key `"Board 3BV: %1"` removed (with its
  9 hand translations); new key `"Partial 3BV: %1 / %2 · 3BV/s: %3"`
  added with 9 fresh hand translations modelled on the existing
  `"3BV: %1 · 3BV/s: %2"` win-dialog conventions for the per-locale
  `/s` suffix:
  - TR `Kısmi 3BV: %1 / %2 · 3BV/sn: %3`
  - ES `3BV parcial: %1 / %2 · 3BV/s: %3`
  - FR `3BV partiel : %1 / %2 · 3BV/s : %3`
  - DE `Teil-3BV: %1 / %2 · 3BV/s: %3`
  - RU `Частичный 3BV: %1 / %2 · 3BV/с: %3`
  - PT `3BV parcial: %1 / %2 · 3BV/s: %3`
  - ZH `部分 3BV：%1 / %2 · 3BV/秒：%3`
  - HI `आंशिक 3BV: %1 / %2 · 3BV/s: %3`
  - AR `3BV الجزئي: %1 / %2 · 3BV/ث: %3`

  All 9 non-en locales 98 finished / 0 unfinished after `lupdate`.
- **Assumptions made:**
  - **Replace, don't append.** The new line's Y subsumes the cycle-
    22 line's value — keeping both would render `Board 3BV: 87 \n
    Partial 3BV: 23 / 87 · 3BV/s: 1.84` with the first line
    redundant. Cost: 9 hand translations of the cycle-22 string
    obsoleted; benefit: loss dialog stays at 6-7 lines instead of
    7-8.
  - **"Cleared 3BV" semantics — Minesweeper-Arbiter convention.** A
    region counts as +1 iff *any* of its cells (zeros or fringe-
    numbered) is opened. Matches the canonical speedrun definition;
    pinned by `testPartialBoardValueRegionCountedWhenAnyCellOpened`
    + `testPartialBoardValueEqualsBoardValueOnWin`.
  - **3BV/s on a loss = partial / elapsed, not total / elapsed.** A
    rate against the whole board's 3BV would imply the player
    cleared the full board at that pace — false. Partial / elapsed
    is the player's actual instantaneous pace; if held to the end
    it linearly extrapolates to the speedrunner's "would finish in"
    estimate.
  - **Sub-tick guard mirroring the win path.** A loss with
    `m_lastElapsedSeconds <= 0.05` would otherwise divide by zero.
    The win path's guard is adopted verbatim
    (`(secs > 0.05) ? (bv / secs) : 0.0`), so a sub-tick boom
    renders `0.00` rather than `inf`/NaN.
  - **Lazy walk over live cells, no incremental tracking.** The
    walk is `O(rows × cols) ≤ 480` (Expert), single isOpened()
    enum compare per cell, runs *exactly once per game* on the
    loss path (or as a postcondition assertion in tests). Cost:
    zero. Cost of incremental tracking: pre-computed `m_cellRegion`
    state + reset paths in three places + risk a future refactor
    of `onCellOpened` desyncs the counter. Same pattern as cycle
    23's `questionMarksPlaced()`.
  - **Two new positional parameters on showEndDialog.** Direct
    parallel of cycles 22/23 reasoning: each independent metric
    gets its own positional parameter so future suppression of one
    doesn't leak into another. Trailing-only addition keeps
    existing call-site argument order stable.
  - **Telemetry tags `partial_bv` (anonymous integer) +
    `partial_bv_per_second` (2-decimal float) on `game.lost`.**
    Parallel to the win-side `bv` / `bv_per_second` tags from
    cycle 14. Same anonymisation shape; same zero PII risk.
- **Skipped:**
  - *Partial-efficiency on the loss dialog.* Efficiency = bv /
    clicks · 100 needs the *full* bv to be meaningful (it's a
    measure of how optimally the player would clear the *whole*
    board). On a partial clear, partial_bv / clicks could be
    misleading because the click count includes any unsatisfied/
    wrong chord clicks. Not comparable to anything the player would
    recognise. Skip — would invite confusion.
  - *Win-dialog parity.* The win dialog already shows
    `3BV: %1 · 3BV/s: %2` (cycle 14). At the win endpoint partialBV
    == boardValue() (anchored by the postcondition test), so a
    partial form on wins would be semantically identical and
    visually noisy. Skip.
  - *Region-aware progress bar / "X of Y openings cleared"
    breakdown.* Two numbers (openings cleared vs isolated cells
    cleared) instead of one — interesting telemetry but not
    actionable for the player. Skip; out of dialog scope.
- **Risks logged:** none. Additive accessor on MineField, additive
  trailing parameters on private `MainWindow::showEndDialog`, the
  `Board 3BV` ↔ `Partial 3BV` swap is a translatable-string
  refactor with both old hand translations being 2 cycles young.
  No QSettings/save-format change, no behavioural change on the
  win or paused paths.
- **Self-review (adversarial pass):**
  - *What breaks in production?* Only failure mode is a future
    refactor that zeroes opened-state on revealAllMines —
    `testPartialBoardValuePreservedOnLoss` pins this exact
    invariant. A second hypothetical: a future change to
    `MineField::cellAt()` returning a clone instead of the live
    button would zero `isOpened()` checks; the test layout walks
    the live grid and would catch this.
  - *Backwards compat?* Additive `MineField` accessor; additive
    trailing positional parameters on private `MainWindow::
    showEndDialog`. Both call sites updated. No public-API
    consumers. The `Board 3BV` → `Partial 3BV` translation swap
    obsoletes 9 fresh strings (all from cycle 22) — no long-
    tenured translations are thrown away.
  - *Error paths?* No new error paths. Sub-tick guard on
    bvPerSecond mirrors the win path's. The `n != nullptr` checks
    in the walk are defence-in-depth (m_buttons is fully populated
    by buildGrid).
  - *Secrets/PII?* `partial_bv` (int [0, 480]) +
    `partial_bv_per_second` (2-decimal float, typically 0–10)
    added to telemetry. Same shape as existing tags.
  - *Performance?* O(rows × cols) ≤ 480 walk on the loss path
    only. Nothing in any hot path; not invoked during
    cell-painting or input handling.
  - *Concurrency?* Single-threaded; no new shared state.
- **Post-release watch (T+~5min):** Sentry
  `karaman/qminesweeper` — `search_issues` for unresolved issues in
  release `qminesweeper@1.27.0` in the last hour returned **zero
  results**. Expected — telemetry is opt-in and the assets just
  published with zero downloads. The signal worth watching for is
  *any* new group tagged with the 1.27.0 release; none observed.
  Watch closed.
- **Next candidates:**
  - **Stats dialog: lifetime "Best %" (partial-clear hall-of-fame)
    for never-won difficulties.** Now that partial-bv is in place,
    a natural next is the *lifetime* counterpart — for difficulties
    a player has never won (Expert for new players, where the
    "Best time" column shows `—` forever), surface the highest
    `safePercentCleared` ever reached on a loss in that slot.
    ~80-line production cost: new Stats methods, Stats fixture
    test, dialog-render branch. Re-parked from cycle 23 and now
    the highest-value remaining feature.
  - **Loss dialog "🎯 Close call!" flair when
    `safePercentCleared >= 90`.** Pure cosmetic, zero new schema,
    one translatable string. Mirrors the win-dialog flair pattern
    (🏆/🏃/🌟/🔥) on the loss side. One-cycle filler.
  - **Win-dialog "Personal best 3BV/s" flair when the run beats the
    per-difficulty best 3BV/s ever recorded.** Parallel to the
    "🏆 New record!" flair (which gates on best-time), but for
    the canonical efficiency metric. Needs a new
    `Stats::recordBestBvPerSecond` accessor + lifetime persistence
    in `~/Library/Preferences/com.mavrikant.QMineSweeper.plist`.

## 2026-04-25 — Cycle 23 — v1.26.0 (autonomous)

- **Chosen problem:** The v1.25.0 loss dialog has six lines —
  `You stepped on a mine.` + duration + percent-cleared + Board 3BV +
  Clicks + Flags placed — covering every player-action and board
  metric *except one*. Right-click cycles `None → Flag → Question →
  None`; v1.24.0 surfaced the user's flag count, but the third step
  (Question) has no analogue. A player who used `?` as a thinking
  aid sees nothing about that activity in the loss recap. The
  cycle-22 log explicitly parked this as the top next candidate
  ("one translatable string, no new state").
- **Evidence:** `MainWindow::onGameLost` reads `userClicks`,
  `flagsPlaced`, `boardValue` from MineField but never queries any
  question-mark state. `MineField` has no question-mark accessor at
  all — Question is purely a per-cell `m_marker == CellMarker::Question`
  state with no aggregate counter (in contrast to `m_flagCount`
  which is tracked on the `flagToggled` signal).
- **Shipped:**
  - Branch: `feat/loss-dialog-question-marks` (squash-merged + deleted)
  - PR: [#44](https://github.com/Mavrikant/QMineSweeper/pull/44)
    (squash-merged as `fc3c9a6`)
  - Release: https://github.com/Mavrikant/QMineSweeper/releases/tag/v1.26.0
  - Release workflow `24931183127` green; all 5 assets +
    `SHA256SUMS.txt` published. Hand-written user-facing release
    notes installed via `gh release edit` — narrates the right-click
    cycle for new readers and explains the gating behaviour
    (line hidden when zero `?` placed; never appears when the
    `?` toggle is off in Settings).
- **Code surface:** ~30 LOC of production diff across
  `mainwindow.{h,cpp}` (read `questionMarksPlaced()` on loss, thread
  to `showEndDialog`, append gated line, telemetry tag) +
  `minefield.{h,cpp}` (new `questionMarksPlaced()` accessor) + 74
  LOC of new tests. The 642/465 line-count number is
  `lupdate`-driven `.ts` line renumbering. Production diff well
  under 30 LOC, total well under the 400-LOC cycle cap.
- **Tests added (5 new, all in `tst_minefield.cpp`):**
  - `testQuestionMarksPlacedZeroBeforeAnyMark` — sanity baseline.
  - `testQuestionMarksPlacedCountsOnlyQuestionCells` — Flag-marked
    cells must not count; only Question-cycled cells.
  - `testQuestionMarksPlacedDecrementsOnCycleAway` — Question →
    None step decrements the count.
  - `testQuestionMarksPlacedPreservedOnLoss` — load-bearing
    regression for the new dialog line. Places `?` on a mined cell
    (cycling None→Flag→Question), steps on the other mine,
    asserts `questionMarksPlaced() == 1` after `revealAllMines`.
    Pins the invariant that `revealAllMines` does not clear
    `m_marker` — any future change that auto-clears the marker on
    reveal would silently zero the dialog metric and trip this test.
  - `testQuestionMarksPlacedResetByNewGame` — newGame() rebuilds
    the grid and zeroes the count.
  - Adversarially verified by zeroing the loop body in
    `questionMarksPlaced()`, rebuilding, and confirming 4 of 5 tests
    fail with the wrong value (the zero-baseline and reset tests
    both expect 0, so they pass trivially against the broken
    implementation — that's a feature, not a bug; it tells me which
    tests are load-bearing for the value path).
- **Translation cost:** 1 new string × 9 hand-translated locales
  (TR, ES, FR, DE, RU, PT, ZH, HI, AR). 0 unfinished per non-en
  locale preserved. Each locale read out:
  - TR `Soru işareti: %1`
  - ES `Signos de interrogación: %1`
  - FR `Points d'interrogation : %1`
  - DE `Fragezeichen: %1`
  - RU `Знаков вопроса: %1`
  - PT `Pontos de interrogação: %1`
  - ZH `问号标记：%1`
  - HI `प्रश्न चिह्न: %1`
  - AR `علامات الاستفهام: %1`
- **Assumptions made:**
  - **Lazy walk over live cells, no separate counter.** Question is
    the third step of the right-click cycle and has no transition
    signal — `flagToggled` only fires on Flag-on / Flag-off. Adding a
    counter would require either a new `questionToggled` signal +
    subscriber wiring in `MineField::wireButton` (parallel to
    flagToggled) OR sweeping the live cells. The walk is
    `O(rows × cols) ≤ 480` (Expert) and runs *exactly once per
    game* on the loss path; the per-cell isQuestion() lookup is a
    single enum compare. Cost: zero. Cost of the alternative:
    +1 signal + 1 slot + new state field + new reset path in three
    places (newGame, newGameReplay, setFixedLayout) + tests for the
    counter invariants. The lazy walk is strictly dominant for
    the access pattern.
  - **Question-marked cells that turn out to be mines DO count.**
    `revealAllMines` paints mined cells with the explosion icon but
    does NOT clear `m_marker` — `isQuestion()` stays true after the
    reveal. Semantically correct: the player did mark the cell with
    `?`, they just guessed wrong about whether it was safe.
    Pinned by `testQuestionMarksPlacedPreservedOnLoss`.
  - **New positional `lossQuestionMarks` parameter on
    `showEndDialog`, not reusing `flagsPlaced` or `lossBoardValue`.**
    Reusing would couple the new gate's semantics to an unrelated
    metric — a future change to suppress flags on certain runs would
    unintentionally flip question marks off. Parallel positional
    parameters keep each metric independently controllable. Direct
    parallel of the cycle-22 reasoning that justified
    `lossBoardValue` as its own parameter.
  - **`> 0` gate on the new line.** Mirrors `flagsPlaced > 0`
    exactly. A common no-`?` loss (especially fast booms) shouldn't
    render a noisy `Question marks: 0`. If a player has the `?`
    toggle off entirely (the `MineButton::questionMarksEnabled`
    static-state setting), `cycleMarker` skips Question and the
    count is structurally always 0 — the gate elides the line
    automatically with no extra branching.
  - **Telemetry tag `qmarks` (anonymous integer) on `game.lost`.**
    Parallel to existing `clicks`/`flags`/`bv` tags from cycles
    20/21/22. Same anonymisation shape; same zero PII risk.
- **Skipped:**
  - *Question-mark count on the win dialog.* Win-path's
    `flagAllMines` does not touch question marks — the count would
    be the user's true `?` count at the moment of victory. But on
    a win, every mine is auto-revealed (well, auto-flagged) and any
    `?` the player still had on the board is irrelevant — they were
    a thinking aid for play, not a celebration. Surfacing them on
    wins would reward indecision. Skip.
  - *Adding the line on the pause overlay.* The pause overlay is
    not a result dialog; it has no result-recap section. Out of
    scope.
  - *`Flags + Question marks: %1 / %2` combined line.* Would save
    one `\n` but cost a second `tr()` key for a layout change,
    invalidating nine existing hand translations of `Flags placed:
    %1`. Net negative. Skip.
- **Risks logged:** none. Additive — no QSettings/save-format
  change, no behavioural change on the win or paused paths, no
  public API surface removed. The `qmarks` telemetry tag is
  anonymous integer, parallel to the existing `clicks`/`flags`/`bv`
  tags from cycles 20/21/22.
- **Self-review (adversarial pass):**
  - *What breaks in production?* Only failure mode is a future
    refactor that clears `m_marker` during `revealAllMines` — the
    `testQuestionMarksPlacedPreservedOnLoss` test pins this exact
    invariant. A second hypothetical: a future `MineField` accessor
    or signal that mutates `m_buttons` between gameLost emission
    and the showEndDialog call (the value is read inline, no async
    gap, so this isn't currently reachable).
  - *Backwards compat?* Additive method on `MineField`; additive
    positional parameter on private `MainWindow::showEndDialog`.
    Both call sites updated. No public-API consumers.
  - *Error paths?* No new error paths. The `btn != nullptr` guard
    in the loop is defence-in-depth — `m_buttons` is built by
    `buildGrid` to be fully populated, but the guard costs nothing
    and matches the pattern in `clearAllQuestionMarks` /
    `freezeAllCells`.
  - *Secrets/PII?* `qmarks` count added to telemetry — anonymous
    integer in the [0, 480] range. Same shape as existing tags.
  - *Performance?* `O(rows × cols) ≤ 480` walk fired exactly once
    per game on the loss path. Nothing in any hot path.
  - *Concurrency?* Single-threaded; no new shared state.
- **Post-release watch (T+~5min):** Sentry
  `karaman/qminesweeper` — `search_issues` for unresolved issues in
  release `qminesweeper@1.26.0` in the last hour returned **zero
  results**. Expected — telemetry is opt-in and the assets just
  published with zero downloads. The signal worth watching for is
  *any* new group tagged with the 1.26.0 release; none observed.
  Watch closed.
- **Next candidates:**
  - **Partial 3BV on the loss dialog.** Now that the static board
    3BV (cycle 22) is shipped, the natural next step is the
    *partial* — `Partial 3BV: X / Y · 3BV/s: Z` so a speedrunner
    sees their per-second pace at the moment of explosion. Multi-
    cycle (~50 LOC region-walk for partial 3BV computation) but the
    most informative loss line we have. Park as the highest-impact
    next big feature.
  - **Stats dialog: lifetime "Best %" (partial-clear hall-of-fame).**
    For never-won difficulties (Expert for new players), surface
    the best safe-percent ever reached on a loss in place of the
    `—` in the "Best time" column. ~80-line production cost.
  - **Loss dialog "🎯 Close call!" flair when `safePercentCleared >=
    90`.** Pure cosmetic, zero new schema, one new translatable
    string. Mirrors the win-dialog flair pattern (🏆/🏃/🌟/🔥)
    but on the loss side. Smallest-of-small candidate, likely a
    one-cycle filler if the bigger features are blocked.

## 2026-04-25 — Cycle 22 — v1.25.0 (autonomous)

- **Chosen problem:** The v1.24.0 loss dialog reads
  `You stepped on a mine.` + `You survived for %1.` +
  `You cleared %1% of the board.` + `Clicks: %1` + `Flags placed: %1` —
  five lines, all about the player's actions. The dialog says nothing
  about the *board itself*. A run on a small Beginner field with
  3BV=12 and a run on an unusually open Expert layout with 3BV=180
  are visually indistinguishable from the recap.
- **Evidence:** `MainWindow::onGameWon` already calls
  `ui->mineFieldWidget->boardValue()` and renders `3BV: %1 · 3BV/s: %2`
  on wins. `MainWindow::onGameLost` ignores the same accessor. The
  data is already cached; the loss path simply discards it.
- **Shipped:**
  - Branch: `feat/loss-dialog-3bv` (squash-merged + deleted)
  - PR: [#43](https://github.com/Mavrikant/QMineSweeper/pull/43)
    (squash-merged as `155acac`)
  - Release: https://github.com/Mavrikant/QMineSweeper/releases/tag/v1.25.0
  - Release workflow `24930187398` green; all 5 assets +
    `SHA256SUMS.txt` published. Asset sizes parallel v1.24.0 to within
    a few KB: Linux AppImage 35.9 MB / tar.gz 35.6 MB, macOS .dmg 23.2
    MB, Windows .zip 44.1 MB, SHA256SUMS.txt 394 B. Hand-written
    user-facing release notes installed via `gh release edit`.
- **Code surface:** ~10 lines of production diff in `mainwindow.{h,cpp}`
  (read `boardValue()` on loss, thread to `showEndDialog`, append
  gated line, telemetry tag) + 1 new translatable string × 9
  hand-translated locales + 14 lines for the new
  `testBoardValuePreservedOnLoss` test. The big diff number is `.ts`
  line renumbering by `lupdate`. Production diff well under 30 LOC.
- **Tests added:**
  - `testBoardValuePreservedOnLoss` — load-bearing regression for
    the `> 0` gate. Places two fixed mines on a 3×3, captures
    `boardValue()` pre-loss, triggers a left-click on the unflagged
    mine, asserts both `boardValue() == initialBV` and
    `initialBV >= 1`. Verified adversarially by replacing
    `return m_boardValue` with `return 0`, rebuilding, confirming
    the test fails on the `initialBV >= 1` assert, then restoring.
  - All 6 ctest suites green pre-push and on all three platform CI
    runs.
- **Translation cost:** 1 new string × 9 hand-translated locales
  (TR, ES, FR, DE, RU, PT, ZH, HI, AR). 0 unfinished per non-en
  locale preserved. Each locale read out:
  - TR `Tahta 3BV: %1`
  - ES `3BV del tablero: %1`
  - FR `3BV du plateau : %1`
  - DE `Spielfeld-3BV: %1`
  - RU `3BV поля: %1`
  - PT `3BV do tabuleiro: %1`
  - ZH `棋盘 3BV：%1`
  - HI `बोर्ड 3BV: %1`
  - AR `3BV اللوحة: %1`
- **Assumptions made:**
  - **Static `Board 3BV: %1`, no `3BV/s` rate suffix.** A
    partial-clear rate would compute `boardValue / m_lastElapsedSeconds`
    — but `boardValue` is the **whole** board's 3BV, while a loss only
    completed a fraction. Reporting that ratio would imply the user
    cleared the entire board's 3BV at that pace, which is false.
    Static value is exact; rate is misleading. Direct parallel of the
    cycle-20 reasoning that justified `Clicks: %1` (without
    Efficiency) on the loss dialog.
  - **Fresh `tr("Board 3BV: %1")` key, not splitting the existing
    win-dialog `tr("3BV: %1 · 3BV/s: %2")`.** Splitting would
    invalidate nine existing hand translations and gain only
    cosmetic dedup. The `Board` prefix also distinguishes the value
    from the rate-paired form for any player who reads both
    dialogs across runs.
  - **New positional `lossBoardValue` parameter on `showEndDialog`,
    not reusing the existing `boardValue` parameter.** Reusing
    would couple the loss-side gate to the win-side intent — a
    future win-side change to suppress the rate line on certain
    runs (e.g. replays) would unintentionally flip the loss-side
    line off. Parallel positional parameters keep the win and loss
    paths independently controllable.
  - **`lossBoardValue > 0` gate.** Mirrors the existing
    `boardValue > 0` win-side gate. Real play can't reach a loss
    with `m_boardValue == 0` (mine placement, which calls
    `compute3BV()`, runs synchronously on first click — well before
    any explosion is reachable). The gate matters only for
    `setFixedLayout`-with-zero-mines test paths.
- **Skipped:**
  - *Partial 3BV (3BV of the revealed area only).* Most informative
    loss line conceivable, but requires a new region-walking
    accessor mirroring `compute3BV()` limited to opened cells.
    Cycle-20 already costed this at ~50 LOC of production code.
    Over budget for one cycle and orthogonal to surfacing the
    static board value. Park.
  - *Adding the line on the win dialog too.* Win dialog already says
    `3BV: %1 · 3BV/s: %2`. Reflowing for symmetry would
    re-translate every locale for zero player benefit.
  - *"🏅 Hard board!" flair when `boardValue` lies in some high
    percentile of the difficulty's expected BV.* Needs per-difficulty
    BV distribution data (multi-cycle collection); user-visible
    confidence claim Sentry can't audit. Skip.
- **Risks logged:** none. Additive — no QSettings/save-format change,
  no behavioural change on the win or paused paths, no public API
  surface removed. The new `bv` telemetry tag is anonymous integer,
  parallel to the existing `clicks` and `flags` tags from v1.23/v1.24.
- **Self-review (adversarial pass):**
  - *What breaks in production?* Only failure mode is a future
    refactor that defers BV computation past the loss path or
    zeroes `m_boardValue` on loss. The new test pins both.
  - *Backwards compat?* Additive method, positional parameter added
    to private `showEndDialog`. Both call sites updated. No
    public-API consumers.
  - *Error paths?* No new error paths.
  - *Secrets/PII?* `bv` count added to telemetry — same shape as
    existing `clicks`/`flags` tags. Anonymous integer.
  - *Performance?* O(1) read of a cached int; nothing in any hot
    path.
  - *Concurrency?* Single-threaded; no new shared state.
- **Post-release watch (T+~5min):** Sentry
  `karaman/qminesweeper` — `search_issues` for unresolved issues in
  release `qminesweeper@1.25.0` in the last hour returned **zero
  results**. Expected — telemetry is opt-in and the assets just
  published with zero downloads. The signal worth watching for is
  *any* new group tagged with the 1.25.0 release; none observed.
  Watch closed.
- **Next candidates:**
  - **Question-marks count on the loss dialog** — for users who use
    `?` as a thinking aid; mirrors flags-placed. One translatable
    string, no new state. The cell-marker tri-state already tracks
    Question; a `MineField::questionMarksPlaced()` getter exposing
    a new counter would be the cycle's only API addition.
  - **Partial 3BV on the loss dialog** — `Partial 3BV: X / Y · 3BV/s:
    Z` so a speedrunner can see their per-second pace at the moment
    of explosion. Multi-cycle (~50 LOC region-walk) but the most
    informative loss line we have. Park.
  - **Stats dialog: lifetime "Best %" (partial-clear hall-of-fame).**
    For never-won difficulties (Expert for new players), surface the
    best safe-percent ever reached on a loss in place of the `—` in
    the "Best time" column. Adds one QSettings field, one
    `recordLoss` call-site change, one stats-dialog cell fallback.
    ~80-line production cost.

## 2026-04-25 — Cycle 21 — v1.24.0 (autonomous)

- **Chosen problem:** The v1.23.0 loss dialog surfaces three player-state
  metrics — duration, percent-cleared, clicks — but says nothing about
  flag-placement. A player who flagged eight mines correctly before
  stepping on the ninth gets the same line as a player who placed zero
  flags and clicked into a mine immediately. Right-click is half the
  player's input vocabulary; the loss recap erases that half.
- **Evidence:** `MainWindow::onGameLost` reads `userClicks` and passes
  it to `showEndDialog` for the v1.23.0 line, but never queries
  flag-placement state. `MineField` already tracks `m_flagCount` for
  `remainingMines()` (the LCD display) — the data exists, the loss
  dialog just doesn't see it.
- **Shipped:**
  - Branch: `feat/loss-dialog-flags-placed`
  - PR: [#42](https://github.com/Mavrikant/QMineSweeper/pull/42)
    (squash-merged as `39dca4f`)
  - Release: https://github.com/Mavrikant/QMineSweeper/releases/tag/v1.24.0
  - Release workflow `24929128997` green; all 5 assets +
    `SHA256SUMS.txt` published. Hand-written user-facing release notes
    installed via `gh release edit` (the auto-generated body is
    raw commit-list noise; the rewritten copy explains *why* a player
    reading the recap should care about flag count).
- **Code surface:** ~1 line getter on `MineField` + ~6 lines in
  `mainwindow.cpp` (read flag count, thread parameter, append gated
  line, telemetry tag) + 5 new `tst_minefield` regression tests + 1 new
  translatable string × 9 hand-translated locales. Production diff well
  under 20 LOC; the rest is `.ts` line-renumbering by `lupdate`.
- **Tests added:**
  - `testFlagsPlacedZeroBeforeAnyFlag` — sanity baseline.
  - `testFlagsPlacedIncrementsOnFlag` — `cycleMarker()` None→Flag bumps
    the count.
  - `testFlagsPlacedDecrementsOnUnflag` — Flag→Question (the cycle that
    *removes* the flag) correctly decrements.
  - `testFlagsPlacedPreservedOnLoss` — the load-bearing regression for
    the new dialog line: places one flag on a two-mine board, steps on
    the unflagged mine, asserts `flagsPlaced() == 1` post-`gameLost`.
    Pins the invariant that the loss path's `revealAllMines()` does not
    auto-flag (only the win path's `flagAllMines()` does), so any
    future change that auto-flags on loss would inflate the dialog
    line and trip this test.
  - `testFlagsPlacedResetByNewGame` — `newGame()` zeroes the counter.
  - Verified adversarially by replacing `return m_flagCount` with
    `return 999` and confirming all five tests fail with the wrong
    value, then restoring.
- **Translation cost:** 1 new string × 9 hand-translated locales (TR,
  ES, FR, DE, RU, PT, ZH, HI, AR). 50/50 finished/unfinished coverage
  preserved. Each locale read out:
  - TR `Yerleştirilen bayrak: %1`
  - ES `Banderas colocadas: %1`
  - FR `Drapeaux placés : %1`
  - DE `Platzierte Flaggen: %1`
  - RU `Установлено флагов: %1`
  - PT `Bandeiras colocadas: %1`
  - ZH `已放置旗子：%1`
  - HI `लगाए गए झंडे: %1`
  - AR `الأعلام الموضوعة: %1`
- **Assumptions made:**
  - **Loss dialog only.** The win path's `flagAllMines()` auto-flags
    every remaining mine — surfacing the post-celebration count would
    mislead. Documented in the new `flagsPlaced()` doc and pinned by
    `testFlagsPlacedPreservedOnLoss` (loss path) — there is no analogue
    test on the win path because we deliberately do not surface the
    metric there.
  - **`flagsPlaced > 0` gating.** Mirrors `userClicks > 0` from v1.23
    and `boardValue > 0` from v1.14/v1.15. Avoids `Flags placed: 0`
    noise on the most common loss (first click is a mine, no flags
    placed yet).
  - **Single-number rendering, not "X / total mines".** Spoils the mine
    count for users who don't memorise it (especially on Custom
    difficulty); also keeps the line shape parallel to `Clicks: %1`.
  - **Direct `flagsPlaced()` getter, not derived from
    `remainingMines()`.** Inverting the formula in dialog code would
    couple presentation to a MineField formula. One-line getter on the
    underlying counter is cleaner.
- **Skipped:**
  - *Showing correct-flag breakdown `Flags: X (Y correct)`.* Doable but
    requires a board walk on loss (count flags-on-mines vs.
    flags-on-safe-cells) and a second translatable string. The
    revealed board already shows wrong-flag overlays via
    `revealAsWrongFlag`; the count addition would surface the same
    info textually for a doubled translation cost. Park.
  - *Adding the line on the win dialog.* Auto-flag inflation makes the
    metric uninteresting (always == mineCount).
  - *Flag-rate (flags/sec) analogue of 3BV/s.* Not a recognised
    Minesweeper community metric, lossy on short runs.
- **Risks logged:** none. Additive — no QSettings/save-format change,
  no behavioural change on the win or paused paths, no API surface
  removed. The new `flagsPlaced()` getter exposes existing state.
- **Self-review (adversarial pass):**
  - *What breaks in production?* The only failure mode is the loss
    path one day auto-flagging; the new test pins this.
  - *Backwards compat?* Additive method; positional parameter added to
    private `showEndDialog`. No public-API consumers.
  - *Error paths?* No new error paths.
  - *Secrets/PII?* `flags` count added to telemetry — same shape as
    existing `clicks` tag. Anonymous integer.
  - *Performance?* O(1) read of an int counter; nothing in any hot
    path.
  - *Concurrency?* Single-threaded; no new shared state.
- **Post-release watch (T+~5min):** Sentry
  `karaman/qminesweeper` — `search_issues` for unresolved issues in
  release `qminesweeper@1.24.0` in the last hour returned **zero
  results**. Expected — telemetry is opt-in and the assets just
  published with zero downloads. The signal worth watching for is
  *any* new group tagged with the 1.24.0 release; none observed.
  Watch closed.
- **Next candidates:**
  - **3BV: %1 line on the loss dialog** — surfaces the *board's*
    objective difficulty as a 5th line, complementing the four
    *player-action* metrics. One translatable string, no new state.
  - **Question-marks count on the loss dialog** — for users who use
    `?` as a thinking aid, mirrors flags-placed.
  - **"Best run" highlight on the loss dialog when this loss beats a
    previous unfinished-run record** — would need a new persisted
    record schema; multi-cycle.

## 2026-04-25 — Cycle 20 — v1.23.0 (autonomous)

- **Chosen problem:** The v1.22.0 loss dialog reads
  `You stepped on a mine.` + `You survived for %1.` +
  `You cleared %1% of the board.` — duration and progress, but no
  effort. A user who reaches 60% in 25 clicks vs. 60% in 80 clicks
  gets the same line and has no way to compare their own efficiency
  across runs.
- **Evidence:** `MainWindow::onGameLost` (the post-v1.22.0 path)
  passes `0` for `userClicks` to `showEndDialog`. `MineField` already
  maintains `m_userClicks` for the win-dialog `Clicks: %1 ·
  Efficiency: %2%` line; the loss path simply discards it.
- **Shipped:**
  - Branch: `feat/loss-dialog-clicks` (squash-merged + deleted)
  - PR: [#41](https://github.com/Mavrikant/QMineSweeper/pull/41)
    (squash-merged as `6989c36`)
  - Release: https://github.com/Mavrikant/QMineSweeper/releases/tag/v1.23.0
  - Release workflow `24928080651` green; all 5 assets +
    `SHA256SUMS.txt` published. Hand-written user-facing release notes
    installed via `gh release edit`.
- **Code surface:** +606 / −422 over 15 files. The big number is nine
  `.ts` translation file rewrites (line numbers shifted by lupdate);
  production code is +13 lines in `mainwindow.cpp` (capture clicks,
  thread to `showEndDialog`, append the gated line, add to telemetry
  tags) + 1 new translatable string × 9 hand-translated locales + 19
  lines for the new `tst_minefield` test.
- **Tests added:**
  - `testUserClicksLeftClickOnMineCountsAndExplodes` — direct
    left-click on a mine increments `userClicks` before the explosion
    freezes the board (the most common loss path; the new dialog line
    depends on this invariant). Existing
    `testUserClicksChordWrongFlagCountsAndExplodes` already covers the
    chord-into-mine variant.
  - All 6 ctest suites green pre-push and on all three platform CI
    runs.
- **Translation cost:** 1 new string × 9 locales (TR, ZH, HI, ES, AR,
  FR, RU, PT, DE). Each locale's translation matches the prefix of
  the existing `Clicks: %1 · Efficiency: %2%` translation, so
  translators get a coherent unit and the loss-dialog `Clicks: 47`
  reads identical to the win-dialog `Clicks: 47 · …` for the same run.
- **Assumptions made:**
  - **Click count, not 3BV/s.** Cycle 19's next-candidate list
    proposed mirroring the win-dialog 3BV/s line. Doing that
    literally would compute `boardValue / m_lastElapsedSeconds` —
    but `boardValue` caches the **whole** board's 3BV at mine
    placement, while a loss only completed a fraction of it.
    Reporting that ratio would imply the user cleared the entire
    board's 3BV at that rate, which is false. Click count is exact,
    requires no new computation, and is the half of the win-dialog
    metric pair that survives unmodified to a loss.
  - **Separate `tr("Clicks: %1")` key, not a split of the existing
    `Clicks: %1 · Efficiency: %2%`.** Splitting would invalidate nine
    existing translations and give translators a less coherent unit.
    The marginal cost of one new key was lower.
  - **`userClicks > 0` gate.** Mirrors the win-dialog gate. Only
    pathological `setFixedLayout` test paths can hit a zero-click
    loss; gating keeps `Clicks: 0` off the user-visible path.
- **Skipped:**
  - *Partial 3BV computation (the 3BV of the revealed area).* Would
    let us also surface efficiency on a loss but requires walking the
    revealed openings + numbered cells with the same connectivity
    rules as `compute3BV()` limited to opened cells. Real
    implementation cost beyond a one-line dialog patch; deferred.
  - *Loss-specific efficiency metric `safe_revealed / clicks · 100`.*
    Plausible but a new metric the user has to learn vs. the win-side
    definition. One cycle, one line — kept it crisp.
  - *Prose form `tr("You used %1 clicks.")`.* Considered for prose
    consistency with the prior loss-dialog lines. Rejected because
    `1 click` singular handling needs `%n` plural forms across nine
    locales and the terse `Clicks: %1` format already has translator
    history from the win dialog.
- **Risks logged:** none. Additive change — `MineField` API surface
  unchanged, no QSettings/save-format change, no behavioural change on
  the win path.
- **Codacy CI:** Codacy Static Code Analysis returned
  `action_required` with no summary — likely a false positive on the
  large `.ts` rewrite by `lupdate`. The four required gates (Build×3
  + Format) all green; coverage didn't drop (one new test added).
  Same Codacy posture as past PRs that merged with mixed Codacy
  results.
- **Post-release watch (T+5min):** Sentry reports 0 unresolved issues
  in release `qminesweeper@1.23.0` (expected — all 5 assets have
  download_count=null/0, no users yet within the watch window). Asset
  sizes match v1.22.0 exactly: Linux AppImage 35.9 MB / tar.gz 35.6
  MB, macOS .dmg 23.2 MB, Windows .zip 44.1 MB, SHA256SUMS.txt 394 B.
- **Next candidates:**
  - *Loss dialog 5th line: partial 3BV.* Compute the 3BV of the
    revealed area and surface `Partial: X / Y · 3BV/s: Z` so a
    speedrunner can see their per-second pace at the moment of
    explosion. Needs a new `MineField::partialBoardValue()` accessor
    and a region-walk that mirrors `compute3BV()` but is limited to
    revealed cells. ~50-line production cost; non-trivial.
  - *Stats dialog: lifetime "Best %" (partial-clear hall-of-fame).*
    For never-won difficulties (especially Expert for new players),
    show the best safe-percent ever reached on a loss in place of the
    `—` in the "Best time" column. Adds one QSettings field
    (`best_safe_percent` + date), one `recordLoss(name, percent)`
    call-site change, one stats-dialog cell fallback. ~80-line
    production cost.
  - *Replay-same-layout completion.* The local feat branch from v1.x
    is stale but the underlying primitive (`MineField::newGameReplay`,
    `canReplay`) shipped in v1.x. Reproducible boards for
    sharing/comparing runs is a frequently-requested Minesweeper
    feature; revisiting the branch is the obvious larger investment.

## 2026-04-25 — Cycle 19 — v1.22.0 (autonomous)

- **Chosen problem:** The v1.21.0 loss dialog now reads
  `You stepped on a mine.` + `You survived for 1:23.` — duration but
  no progress. A player who exploded after revealing 87% of the safe
  cells gets the same one-liner as a player who misclicked at 12%.
  The "almost won" feeling has no in-app surface.
- **Evidence:** `MainWindow::showEndDialog()` (loss branch, the post-
  v1.21.0 path) only shows duration. `MineField` already maintains
  `m_openedSafeCount` for win-detection but never exposes it. Pure
  inspection of the existing code — no telemetry needed for a
  user-facing "did I almost make it?" gap.
- **Shipped:**
  - Branch: `feat/loss-dialog-percent-cleared` (merged + deleted)
  - PR: https://github.com/Mavrikant/QMineSweeper/pull/40
    (squash-merged as `608f954`)
  - Release: https://github.com/Mavrikant/QMineSweeper/releases/tag/v1.22.0
  - Release workflow `24926952738` green; all 5 assets +
    `SHA256SUMS.txt` published (~2 min total). Hand-written
    user-facing release notes installed via `gh release edit`.
- **Code surface:** +329 / −191 over 16 files. Production code:
  `MineField::safePercentCleared()` (10 lines in `.cpp`, 7-line
  doxygen-style comment in `.h`) and a single `tr()` line appended
  in `MainWindow::showEndDialog()`. The rest is 4 unit tests + 9
  hand-translated locale strings + the `apply_translations.py`
  entries.
- **Tests added:** `testSafePercentClearedZeroBeforeAnyClick`,
  `testSafePercentClearedReachesHundredOnWin`,
  `testSafePercentClearedMidGame` (50% mid-game),
  `testSafePercentClearedRoundsHalfUp` (1/3→33, 2/3→67 boundary).
  All 6 ctest suites green pre-push and on all three platform CI runs.
- **Translation cost:** 1 new string × 9 locales (TR, ES, FR, DE,
  RU, PT, ZH, HI, AR). The previous cycle's
  `apply_translations.py` was missing HI and AR entries when the
  feature work landed in the working tree — caught by manual
  inspection of `apply_translations.py`'s coverage and added
  before commit. All 10 `.ts` files now show
  `<translation>...</translation>` (no `unfinished`) for the new key.
- **Assumptions made:**
  - **Round-half-up via integer arithmetic** —
    `(opened * 100 + total/2) / total`. Stops a 99.5% near-win from
    rendering as "99%". `Round-half-to-even` (banker's rounding) is
    statistically nicer but not what users expect from a UI metric.
  - **Loss-only line** — a win is implicitly 100%, surfacing the
    line on a win is noise; the existing "You won!" path stays
    untouched.
  - **Hindi and Arabic translations are correct.** Hand-modeled on
    the existing "You cleared the field" idiom in each locale; no
    machine translation.
- **Skipped:**
  - *Cell-count form ("87 of 99 cells")* — noisier, less satisfying,
    same information. Percent is the right precision for an end-of-
    game pat-on-the-back.
  - *Reuse the win-dialog `cleared in %1` line* — different semantics
    (time vs. progress), different verb tense ("cleared" vs.
    "have cleared X% of"). Kept separate strings for translation
    fidelity.
- **Risks logged:** none. Additive `MineField` API, no QSettings or
  save-format change, no behavioural change on the win path.
- **Post-release watch (T+10min):** Sentry shows 0 unresolved issues
  in release `qminesweeper@1.22.0` — expected zero (all 5 assets
  have download_count=0; no users yet). Asset sizes look sane:
  Linux AppImage 35.9 MB, tar.gz 35.6 MB, macOS .dmg 23.2 MB,
  Windows .zip 44.1 MB, SHA256SUMS.txt 394 B. No regression
  signal within the window this cycle could realistically cover.
- **Next candidates:**
  - Loss dialog: 4th line surfacing the run's 3BV efficiency at the
    moment of explosion (mirrors the win dialog's 3BV/s line) — same
    "didn't quite make it but here's what you accomplished" theme.
  - Stats dialog: lifetime "Best %" for a partial-clear hall-of-fame
    when no win exists yet on a difficulty (would let new players
    on Expert see *some* personal record before their first win).
  - Replay-same-layout: branch already exists locally; reproducible
    boards for sharing/comparing runs is a frequently-requested
    Minesweeper feature and the local feat branch is the obvious
    next investment.

## 2026-04-25 — Cycle 18 — v1.21.0 (autonomous)

- **Chosen problem:** The end-of-game **Boom** dialog still read just
  `You stepped on a mine.` — no run length, no context. After v1.18.0
  (live timer), v1.19.0 (stats best-time columns), and v1.20.0 (win
  dialog) all migrated their duration display onto the
  `formatElapsedTime` helper (`S.S` / `M:SS.S` / `H:MM:SS.S`), the
  loss dialog was the only duration-relevant surface that still
  showed nothing at all. Cycle 17 explicitly parked nothing here, but
  the parity gap was the cleanest visible dangler in the loss UX —
  players had no way to gauge how far into a run their mistake landed.
- **Evidence:** `mainwindow.cpp:821` — `box.setText(tr("You stepped on
  a mine."));` is the entire body text of the loss dialog. The win
  dialog directly above it composes a multi-line `text` with
  `formatElapsedTime(m_lastElapsedSeconds)`. `m_lastElapsedSeconds` is
  already populated by `onGameLost` for the toolbar refresh; nothing
  new to thread.
- **Shipped:**
  - Branch: `feat/loss-dialog-duration` (squash-merged + deleted)
  - PR: [#39](https://github.com/Mavrikant/QMineSweeper/pull/39)
  - Squash commit: `c71150b`
  - Tag: `v1.21.0`
  - Release: [v1.21.0](https://github.com/Mavrikant/QMineSweeper/releases/tag/v1.21.0)
- **Diff shape:** 13 files, +243 / -182. Semantic change is **3 LOC**
  in `mainwindow.cpp` (compose a two-line `text`, append the new
  `formatElapsedTime` line, set on the box), **1 LOC** in
  `CMakeLists.txt` (1.20.0 → 1.21.0), and **9 LOC** in
  `translations/apply_translations.py` (one entry per hand-locale).
  Remainder is `lupdate` rewriting line-number metadata across all 10
  `.ts` files for the new key. Well under the 400-LOC cycle cap.
- **Format chosen:** identical to win dialog.
  - `< 60 s` → `S.S` (`You survived for 45.0.`)
  - `60..3600 s` → `M:SS.S` (`You survived for 12:34.5.`)
  - `≥ 1 h` → `H:MM:SS.S` (`You survived for 1:00:00.5.`)
  - First-click loss on replay: `m_lastElapsedSeconds` is small but
    positive (the press emitted `gameStarted` before the explosion);
    renders as `0.0` or e.g. `0.1`. Internally consistent with the
    live timer reading at the moment of explosion.
- **Translation cost:** 1 new hand-translated string × 9 non-English
  locales. `lupdate` reports **93 finished / 0 unfinished** across
  every hand-locale. Existing `You stepped on a mine.` translation
  preserved verbatim — the duration line is appended on a new line,
  not substituted.
- **Assumptions made:**
  - **Append, don't replace.** Replacing the message would force a
    rewrite of `You stepped on a mine.` across 9 hand-locales for
    cosmetic prose elegance. Two short sentences read just as well
    and cost zero churn on the existing translation.
  - **Always show the line, even when zero.** First-click loss on
    replay yields `0.0`-ish; consistent with how the live timer
    reads at the same moment. A guard (`if elapsed > 0.05`) would
    make the dialog inconsistent across runs.
  - **No persistence change.** No new QSettings keys, no schema bump.
    Only the rendering path changes.
  - **No new test scaffold.** Identical risk profile to v1.18 / v1.19
    / v1.20 — single tr() call into a helper already pinned by 14
    deterministic test cases in `tst_time_format`. A `MainWindow`-
    level dialog test would cost more in scaffolding than it earns.
  - **"You survived for"** vs **"Time:"**. Mirrors the win dialog's
    full-sentence register (`You cleared the field in %1.`). A flat
    label clashes with the rest of the dialog body.
- **Skipped:**
  - *Hide duration on first-click loss.* See above — would create
    inconsistent reads across runs and add a guard for a one-edge-case
    aesthetic.
  - *Show "% of board cleared" on loss.* Tempting feedback for
    learning, but adds another translatable string and another
    rendering branch. Not this cycle.
  - *Show 3BV / clicks / efficiency on loss.* These metrics are
    win-only by definition (3BV is "minimum clicks to **clear**");
    partial completion has no canonical analog.
  - *Hint button (limited per game).* Still parked — needs a small
    deterministic solver and a ~250-400 LOC slice. Multi-cycle.
  - *Save-and-resume across launches.* Still parked — board state +
    marker state + timer offset + QSettings schema bump. Multi-cycle.
- **UI smoke:** `ctest` 6/6 green; `clang-format` clean across all
  `.cpp` / `.h` / `tests/*.cpp`. Compiled Debug bundle on macOS
  (Qt 6.11) without warnings. The `formatElapsedTime` helper is
  already pinned by `tst_time_format` (14 cases including 7.3, 45.0,
  90.5, 754.5, 3600.5, 0.0, -1.0).
- **Risks logged:** none new. No persistence change, no signal/slot
  wiring, no public API change. Worst case is a loss-dialog format
  regression caught by `tst_time_format` if the helper itself drifts.
- **Post-release watch (T+~3 min):** Release workflow
  [run 24924905588](https://github.com/Mavrikant/QMineSweeper/actions/runs/24924905588)
  green across all three platforms in 1m53s (macOS), 1m16s (Windows),
  1m26s (Linux) — total wall-clock ~2 min. Five assets published
  (Linux AppImage, Linux tar.gz, macOS universal DMG, Windows x64
  ZIP, plus `SHA256SUMS.txt`). Sentry `karaman/qminesweeper` —
  `search_issues` for `release:qminesweeper@1.21.0` in the last hour
  returned **zero issues**. Expected: opt-in telemetry, assets fresh,
  no install has had a chance to fire a release-health session yet.
  GitHub release body rewritten from the auto-generated stub to
  user-facing prose covering the new loss-dialog line, per-platform
  downloads, and the macOS quarantine note. Watch closed.
- **Next candidates:**
  - Save-and-resume across launches (still parked; multi-cycle).
  - Hint button (limited per-game; needs a small deterministic
    solver).
  - Daily / "games played today" mini-counter on the win/loss dialog
    — derived from QSettings + QDate; small but burns translation
    churn (~2 strings × 9 locales).
  - Per-layout best-time leaderboard (mine-position hash + new
    persistence schema; multi-cycle).
  - Optional `Settings → Show tenths-of-a-second on the live timer`
    toggle for players who want a steadier `S` / `M:SS` / `H:MM:SS`
    clock without the flicker. Cheap; one new translatable checkbox
    label.
  - "% of board cleared" on the loss dialog — informative for
    learning ("you were 87% there before stepping on it"); one extra
    translatable string × 9 locales.

## 2026-04-25 — Cycle 17 — v1.20.0 (autonomous)

- **Chosen problem:** The end-of-game **You won!** dialog still
  rendered the cleared-time line as `%.1f seconds.` — e.g.
  `You cleared the field in 754.5 seconds.` for a 12-and-a-half
  minute Expert run. After v1.18.0 moved the live toolbar timer
  and v1.19.0 moved the stats-dialog *Best time* / *Best (no
  flag)* columns to the duration-aware `S.S` / `M:SS.S` /
  `H:MM:SS.S` formatter, the win dialog was the only remaining
  surface where elapsed time read in raw decimal seconds.
- **Evidence:** `mainwindow.cpp:781` —
  `tr("You cleared the field in %1 seconds.").arg(QString::asprintf("%.1f", m_lastElapsedSeconds))`.
  The same `formatElapsedTime(...)` helper was already imported
  and used at three other call sites in the same file (live
  label refresh, ready-state reset, stats-dialog formatBest).
  Cycle 15 (v1.18.0) and Cycle 16 (v1.19.0) both explicitly
  parked this as the next candidate, citing the 9-locale
  hand-translation cost as the reason to defer. With both
  earlier cycles now in production and zero Sentry hits across
  two post-release watches, the format contract was proven and
  the parity gap was the cleanest visible dangler.
- **Shipped:**
  - Branch: `feat/win-dialog-mm-ss` (squash-merged + deleted)
  - PR: [#38](https://github.com/Mavrikant/QMineSweeper/pull/38)
  - Squash commit: `b565081`
  - Tag: `v1.20.0`
  - Release: [v1.20.0](https://github.com/Mavrikant/QMineSweeper/releases/tag/v1.20.0)
- **Diff shape:** 13 files changed, +760 / -724 lines, but the
  semantic change is **2 LOC** in `mainwindow.cpp` (call swap
  from `asprintf("%.1f", …)` to `formatElapsedTime(…)` plus
  source-string trim of the trailing `seconds` noun), **1 LOC**
  in `CMakeLists.txt` (version bump 1.19.0 → 1.20.0), and **9
  LOC** in `translations/apply_translations.py` (one entry per
  hand-locale). The remaining churn is `lupdate` rewriting all
  10 `.ts` files — line-number metadata + the new key + the
  `type="vanished"` bookkeeping for the obsolete key. Well
  under the 400-LOC cycle cap once `.ts` regen is excluded.
- **Format chosen:** identical to v1.18.0 / v1.19.0.
  - `< 60 s` → `S.S` (`You cleared the field in 45.0.`)
  - `60..3600 s` → `M:SS.S` (`...in 1:30.5.`,
    `...in 12:34.5.`)
  - `≥ 1 h` → `H:MM:SS.S` (`...in 1:00:00.5.`)
  - Defensive: negative / NaN / inf → formatter returns
    `"0.0"`, dialog reads `...in 0.0.`
- **Translation cost:** real but absorbed. All 9 hand-locales
  (tr, es, fr, de, ru, pt, zh, hi, ar) updated in one pass:
  drop the unit-noun, no verb refactor required. `lupdate`
  reports **92 finished / 0 unfinished** across every locale;
  the previous translation drops to `type="vanished"` per
  `lupdate`'s deprecated-entry bookkeeping.
- **Assumptions made:**
  - **Drop the `seconds` noun, don't substitute it.** A
    colon-clock value reads wrong followed by `s`; matches
    live-timer + stats-column precedent. Mixing `45.0 seconds`
    and `1:30.5` inside one dialog across runs would
    re-introduce the parity gap I came to close.
  - **No persistence change.** Stored `bestSeconds` /
    `bestNoflagSeconds` remain `double` of seconds. Only the
    rendering path changes.
  - **No new test scaffold.** Identical risk profile to v1.18 /
    v1.19 — a one-line call swap to a helper already pinned by
    14 deterministic test cases in `tst_time_format`. Adding a
    `MainWindow`-level dialog test would cost more in
    scaffolding than it earns.
  - **Turkish translation refactor.** Direct word-drop
    (`"Alanı %1 temizlediniz."`) reads ungrammatical without
    a temporal particle. Used the natural locative
    `"Alanı %1 içinde temizlediniz."` ("You cleared the field
    within %1.") — preserves the verb and reads idiomatically
    with a colon-clock placeholder. All other 8 locales took
    a clean unit-noun drop with no verb change.
- **Skipped:**
  - *Lifting the win-dialog text composition into a testable
    helper.* Single call site; the format half is already
    pinned by `tst_time_format` and the translation half is a
    static `tr(...)` lookup. Premature.
  - *Hint button (limited per game).* Still parked — needs a
    small deterministic solver and a ~250-400 LOC slice.
    Multi-cycle work, not this one.
  - *Save-and-resume across launches.* Still parked — board
    state + marker state + timer offset + QSettings schema
    bump. Multi-cycle.
  - *Daily / "games played today" mini-counter.* Still on the
    parking lot but burns translation churn for a small
    surface; not next.
- **UI smoke:** Local `clang++` smoke compile of
  `formatElapsedTime` against the new template
  (`"You cleared the field in %1."`) confirmed exact strings
  for 7.3 / 45.0 / 90.5 / 754.5 / 3600.5 / 0.0 / -1.0:
  `You cleared the field in 7.3.`,
  `...in 45.0.`, `...in 1:30.5.`, `...in 12:34.5.`,
  `...in 1:00:00.5.`, `...in 0.0.`, `...in 0.0.`. Headless
  `ctest` 6/6 green; `clang-format` clean across all `.cpp`
  / `.h`.
- **Risks logged:** none new. No persistence change, no
  signal/slot wiring, no public API change. Worst case is a
  win-dialog format regression caught by the formatter's
  `tst_time_format` if the helper itself drifts.
- **Post-release watch (T+~3 min):** Release workflow
  [run 24923854735](https://github.com/Mavrikant/QMineSweeper/actions/runs/24923854735)
  green across all three platforms; five assets published
  (Linux AppImage 34.2 MiB, Linux tar.gz 33.9 MiB, macOS
  universal DMG 22.1 MiB, Windows x64 ZIP 42.1 MiB, plus
  `SHA256SUMS.txt`). Sentry `karaman/qminesweeper` —
  `search_issues` for `release:qminesweeper@1.20.0` returned
  **zero issues**. Expected: opt-in telemetry, assets fresh,
  no install has had a chance to fire a release-health
  session yet. GitHub release body rewritten from the
  auto-generated stub to user-facing prose covering the new
  win-dialog format, per-platform downloads, and the macOS
  quarantine note. Watch closed.
- **Next candidates:**
  - Save-and-resume across launches (still parked; multi-cycle).
  - Hint button (limited per-game; needs a small deterministic
    solver).
  - Daily / "games played today" mini-counter on the win
    dialog — derived from QSettings + QDate; small but burns
    translation churn.
  - Per-layout best-time leaderboard (mine-position hash + new
    persistence schema; multi-cycle).
  - Optional `Settings → Show tenths-of-a-second on the live
    timer` toggle for players who want a steadier `S` / `M:SS`
    / `H:MM:SS` clock without the flicker. Cheap to ship,
    purely additive, no new translatable strings if framed as
    a checkbox under an existing menu.

## 2026-04-25 — Cycle 16 — v1.19.0 (autonomous)

- **Chosen problem:** The Statistics dialog's *Best time* and
  *Best (no flag)* columns rendered as `%.1f s` (e.g.
  `754.5 s`), forcing the player to mentally divide by 60 to
  reason about a long run. The live toolbar clock had already
  been switched to a duration-aware `S.S` / `M:SS.S` /
  `H:MM:SS.S` formatter in v1.18.0, so the stats column lagged
  the in-game display. v1.18.0's CYCLES entry explicitly listed
  this parity fix as the next candidate after the live-label
  change had aged.
- **Evidence:** `MainWindow::showStatsDialog` lambda
  `formatBest` used `QString::asprintf("%.1f s", seconds)`. No
  call to `formatElapsedTime` from `mainwindow.cpp` outside the
  live-label path.
- **Shipped:**
  - Branch: `feat/stats-dialog-mm-ss` (squash-merged + deleted)
  - PR: [#37](https://github.com/Mavrikant/QMineSweeper/pull/37)
  - Squash commit: `28cb44c`
  - Tag: `v1.19.0`
  - Release: [v1.19.0](https://github.com/Mavrikant/QMineSweeper/releases/tag/v1.19.0)
- **Diff shape:** 2 files, +6/-3 LOC of behavior — the smallest
  cycle so far. Plus `DECISIONS.md` +57 and this CYCLES entry.
  Well under the 400-LOC cycle cap.
- **Translation cost:** zero. The format is composed at runtime
  inside `formatElapsedTime`. All 10 locales stay 90/90
  finished, 0 unfinished.
- **Format chosen:** identical to v1.18.0's live label.
  - `< 60 s` → `S.S` (e.g. `45.0`)
  - `60..3600 s` → `M:SS.S` (e.g. `1:30.5`, `12:34.5`)
  - `≥ 1 h` → `H:MM:SS.S` (e.g. `1:00:00.0`)
  - Negative / NaN / inf → `0.0` (defensive, formatter-side)
- **Assumptions made:**
  - **Drop the `s` unit.** The "Best time" / "Best (no flag)"
    column headers carry the unit; pairing `s` with `1:30.5`
    reads wrong. Other Minesweeper-family games (KMines, GNOME
    Mines) ship without per-cell units in these columns.
  - **No persistence change.** Stored `bestSeconds` /
    `bestNoflagSeconds` remain `double` of seconds. Legacy
    QSettings load identically; only the rendering path
    changes.
  - **No new test scaffold.** The change is a literal call
    swap to a helper that's already pinned by 14 deterministic
    test cases (boundary rounding, hour carry, defensive zero,
    trailing-tenth preservation). Adding a stats-dialog test
    layer would have cost more in scaffolding than it earned
    in coverage.
  - **Lambda's own `seconds <= 0.0` early-return stays.** The
    "no record yet" cell needs to render `"—"`, not `"0.0"`,
    so the rendering still distinguishes "absent" from "zero".
- **Skipped:**
  - *Reformat the win-dialog "You cleared the field in %1
    seconds." line.* Would touch a translated string and burn
    9 hand-translations on a cosmetic change. v1.18.0 already
    rejected this for the same reason; keep the cycle additive.
  - *Lift `formatBest` to a free function in `time_format.h`
    for direct testing.* Adds `QLocale` + `QDate` to the
    header; single call site; the format half is already
    tested in isolation.
  - *Hint button (limited per game).* On the parking lot since
    v1.18.0 — needs a small deterministic solver and a
    ~250-400 LOC slice. Multi-cycle work, not this one.
- **Risks logged:** none new. No persistence change, no
  signal/slot wiring, no public API change. Worst case is a
  stats-dialog format regression; the formatter's behavior is
  pinned by `tst_time_format`.
- **UI smoke:** Deferred — cron-launched task context lacks
  display capture permissions. Full headless `ctest` (6/6)
  green; `clang-format` clean across all `.cpp` / `.h`. The
  single line of behavior change is a literal call swap to a
  tested helper.
- **Post-release watch (T+~3min):** Release workflow
  [run 24922788093](https://github.com/Mavrikant/QMineSweeper/actions/runs/24922788093)
  green across all three platforms; five assets published
  (Linux AppImage 34.2 MiB, Linux tar.gz 33.9 MiB, macOS
  universal DMG 22.1 MiB, Windows x64 ZIP 42.1 MiB, plus
  `SHA256SUMS.txt`). Sentry `karaman/qminesweeper` —
  `search_issues` for release `qminesweeper@1.19.0` returned
  **zero issues**. Expected — opt-in telemetry, assets just
  published, no install has had a chance to fire a session
  yet. GitHub release body rewritten from the auto-generated
  stub to user-facing prose covering the new column format,
  per-platform downloads, and the macOS quarantine note.
  Watch closed.
- **Next candidates:**
  - Save-and-resume games across launches (still parked —
    board-state + marker-state + timer-offset serialization +
    QSettings schema bump).
  - Hint button (limited per-game, exposes 1 safe cell at a
    cost) — needs a small deterministic solver.
  - Daily-streak / "games played today" mini-counter on the
    win dialog — derived from QSettings + QDate, smaller than
    save-and-resume.
  - Per-layout best-time leaderboard (mine-position hash + new
    persistence schema).
  - Reformat the win-dialog "You cleared the field in N
    seconds." line to use clock format — natural follow-up,
    but burns 9 hand-translations.

## 2026-04-25 — Cycle 15 — v1.18.0 (autonomous)

- **Chosen problem:** The toolbar clock label rendered as a
  fixed-width `%05.1f` (`000.0`–`999.9`). Past 999.9 s the
  format spilled to six characters (`1234.5`), nudging the
  toolbar layout, and the player had to mentally divide by 60
  to reason about the elapsed run. With pause/resume in v1.12.0
  and per-difficulty stats already shipped, "how long was that
  run" is a real signal — it just needed a clock-shaped
  display. The MM:SS format was on the Next-candidates list of
  v1.16.0 and v1.17.0 explicitly tagged "small polish, zero
  new translatable strings if the format is composed at
  runtime."
- **Evidence:** `MainWindow::updateTimerLabel()` and
  `MainWindow::resetTimerUi()` both used `%05.1f`. No
  duration-aware formatter anywhere in the tree.
- **Shipped:**
  - Branch: `feat/timer-mm-ss-format` (squash-merged + deleted)
  - PR: [#36](https://github.com/Mavrikant/QMineSweeper/pull/36)
  - Squash commit: `c7f3a00`
  - Tag: `v1.18.0`
  - Release: [v1.18.0](https://github.com/Mavrikant/QMineSweeper/releases/tag/v1.18.0)
- **Diff shape:** 6 files, +242/-3 LOC — productive slice
  ~150 LOC (`time_format.h` 49, `tst_time_format.cpp` 118 with
  14 test cases, `mainwindow.cpp` -2/+2, `tests/CMakeLists.txt`
  +1, `CMakeLists.txt` version + source list +2). Plus
  `DECISIONS.md` +69. Well under the 400-LOC cycle cap.
- **Translation cost:** zero. The label is composed entirely
  with `QString::asprintf` — no `tr()` strings touched, so
  every locale stays at 90/90 finished.
- **Format chosen:**
  - `< 60 s` → `S.S` (e.g. `5.7`, `45.0`)
  - `60..3600 s` → `M:SS.S` (e.g. `1:30.5`, `12:34.5`)
  - `≥ 1 h` → `H:MM:SS.S` (e.g. `1:00:00.0`)
  - Negative / NaN / inf → `0.0` (defensive)
- **Assumptions made:**
  - **Tenths preserved.** The whole speedrun trio (3BV, 3BV/s,
    efficiency) shipped in v1.14–v1.15 already; surrendering
    sub-second precision on the live counter would un-do that
    direction.
  - **Round-then-bucket order.** Format-tenths first, then
    decompose into H/M/S — otherwise `59.97 s` displays as
    `59.9` while still bucketing to "< 60", and the bookkeeping
    drifts. Tests pin `9.97 → 10.0`, `59.97 → 1:00.0`,
    `1:59.97 → 2:00.0`, `59:59.97 → 1:00:00.0`.
  - **Stored values unchanged.** `bestSeconds`,
    `bestNoflagSeconds`, telemetry `duration_seconds`, and the
    win-dialog "you cleared the field in N seconds" line all
    keep decimal-seconds form. No schema migration; legacy
    QSettings load identically.
  - **Stats best-time column unchanged.** It currently pairs
    `42.7 s` with a unit; switching to `1:30.5` removes the
    unit. Defer to a future cycle once the live format has
    aged in production.
  - **Header-only formatter.** Pure inline `QString
    formatElapsedTime(double)` in `time_format.h`. Test file
    needs only the include — no library link change.
- **Skipped:**
  - *Reformat the win-dialog seconds string.* Would touch the
    existing translated string and burn 9 hand-translations on
    a cosmetic change.
  - *Reformat the stats best-time column.* Strips the `s` unit;
    deferred for a "consistency polish" cycle later.
  - *Hard cap at `999.9`-style classic Windows Minesweeper.*
    Hides truth from the player.
  - *Localised `1m 30.5s`-style format.* Adds tr() strings ×
    9 locales for no extra information value over a colon clock.
- **Risks logged:** none new. No persistence change, no
  signal/slot wiring, no public API change. Worst case is a
  format regression in the live label; all 14 boundaries are
  pinned by deterministic unit tests.
- **UI smoke:** Deferred. Cron-launched task context lacks
  display capture permissions. The change is a one-line
  swap of the format-string call; the formatter is exhaustively
  unit-tested.
- **Post-release watch (T+~3min):** Release workflow
  [run 24921768411](https://github.com/Mavrikant/QMineSweeper/actions/runs/24921768411)
  green across all three platforms; five assets published
  (Linux AppImage 35.9 MiB, Linux tar.gz 35.6 MiB, macOS
  universal DMG 23.2 MiB, Windows x64 ZIP 44.1 MiB, plus
  `SHA256SUMS.txt`). Sentry `karaman/qminesweeper` —
  `search_issues` for release `qminesweeper@1.18.0` in the
  last hour returned **zero results**. Expected — opt-in
  telemetry, assets just published, no install has had a
  chance to fire a session yet. GitHub release body rewritten
  from the auto-generated stub to user-facing prose covering
  the new format ranges, per-platform downloads, and the macOS
  quarantine note. Watch closed.
- **Next candidates:**
  - Save-and-resume games across launches (still parked —
    board-state + marker-state + timer-offset serialization +
    QSettings schema bump).
  - Per-layout best-time leaderboard (mine-position hash + new
    persistence schema).
  - Hint button (limited per-game, exposes 1 safe cell at a
    cost) — needs a small deterministic solver.
  - Daily-streak / "games played today" mini-counter on the
    win dialog — derived from QSettings + QDate, smaller than
    save-and-resume.
  - Stats-dialog best-time column also adopting MM:SS for runs
    ≥ 60 s, with a unit-pairing refresh — natural follow-up to
    this cycle's live-label switch.

## 2026-04-25 — Cycle 14 — v1.17.0 (autonomous)

- **Chosen problem:** The classic Minesweeper 1–8 digit palette
  collapses `2/3` (green / red) and `5` (dark red) under red-green
  colour vision deficiency — about 8 % of male players read those
  digits as the same colour. Number reading is the primary input
  modality in Minesweeper, so this is a real accessibility barrier.
  The "Color-blind-friendly cell number palette" has been parked
  three cycles in the Next-candidates list (v1.13 → v1.16) as
  "design-level work"; picking the peer-reviewed Okabe-Ito palette
  removes the design obstacle.
- **Evidence:** `minebutton.cpp`'s `numberColor()` returns the
  classic palette: `2` = `rgb(1,126,0)` (green), `3` = `rgb(251,3,1)`
  (red), `5` = `rgb(125,0,1)` (dark red). Under deuteranopia those
  three colours simulate to nearly identical olive/brown shades. No
  `colorblind` / `colorBlind` / `cvd` anywhere in the tree. GNOME
  Mines and Microsoft Minesweeper both ship a toggle; ours did not.
- **Shipped:**
  - Branch: `feat/colorblind-palette` (squash-merged + deleted)
  - PR: [#35](https://github.com/Mavrikant/QMineSweeper/pull/35)
  - Tag: `v1.17.0`
  - Release: [v1.17.0](https://github.com/Mavrikant/QMineSweeper/releases/tag/v1.17.0)
- **Diff shape:** 21 files, +1052/-682 LOC — productive slice
  ~255 LOC (minebutton +71, minefield +20, mainwindow +25,
  CMakeLists +1, apply_translations +9, tests +139 with 6 new
  cases). The rest is per-locale `.ts` line-rewriting that
  `lupdate` emits for the single new source string, plus the
  DECISIONS entry. Well under the 400-LOC cycle cap.
- **Translation cost:** 1 new source string (`&Color-blind friendly
  numbers`) × 9 hand-translated locales = 9 fresh translations.
  All 10 locales now 90/90 finished, 0 unfinished.
- **Assumptions made:**
  - **Opt-in (default off).** Existing players expect the classic
    palette. Toggle lives at `Settings → Color-blind friendly
    numbers`.
  - **Okabe-Ito palette.** Peer-reviewed, CVD-safe, industry
    standard for categorical data. Reordered so the most common
    digits (1, 2, 3) land on the highest-contrast triad
    (blue / bluish green / vermillion).
  - **App-wide static on `MineButton`.** Mirrors the
    question-marks pattern; `MineButton` has no back-pointer to
    `MineField` by architecture, so app-global settings live as
    static state.
  - **Mid-game toggle repaints opened cells.** New
    `MineField::refreshAllNumberStyles()` sweeps the grid and calls
    `MineButton::refreshNumberStyle()` on each opened numbered
    cell. Silent "takes effect on next reveal" would leave the user
    doubting the toggle worked.
  - **Zero cells skip refresh.** No digit is drawn there, so
    there's nothing to recolour.
  - **Mined cells skip refresh.** Re-styling them would wipe the
    explosion / revealed-mine / wrong-flag loss-state visuals.
  - **QSettings key `settings/colorblind_palette`.** Legacy plists
    with no key read as `false` → classic palette. Zero migration.
- **Skipped:**
  - *Pattern overlays on digits (dots / stripes).* Would read as
    visual noise over a ~22 px glyph in a 30 px cell.
  - *Full high-contrast / dark-mode theme.* Much wider scope —
    every stylesheet (opened cell, mine, wrong-flag, pause overlay,
    smiley button, menu bar) would need theming. Multi-cycle work.
  - *Per-digit custom colour picker.* Fidelity for scope-explosion
    cost; not worth it for the accessibility payoff size.
  - *Auto-detect via OS accessibility settings.* Qt doesn't expose
    a CVD hint on macOS / Windows; heuristic parsing of `QPalette`
    would guess wrong often enough to be worse than an explicit
    opt-in.
- **Risks logged:** none new. The palette addition is additive;
  legacy QSettings load as the classic palette. The refresh helper
  is gated on `m_isClicked && !m_isMined && m_number != 0` so it
  can never wipe loss-state visuals or leak number colours onto
  unopened cells. Tests pin those guards.
- **UI smoke:** Deferred — cron-launched task context lacks display
  capture permissions. Logic is covered by 6 new deterministic
  unit tests (4 in `tst_minebutton` + 2 in `tst_minefield`); the
  palette is a pure function of the static toggle and the digit,
  so visual correctness is an RGB comparison. Manual end-to-end
  verification will happen on the next interactive session if any
  visual regression appears.
- **Post-release watch (T+~2min):** Release workflow
  [run 24920734444](https://github.com/Mavrikant/QMineSweeper/actions/runs/24920734444)
  green across all three platforms; five assets published
  (Linux AppImage, Linux tar.gz, macOS universal DMG, Windows x64
  ZIP, plus `SHA256SUMS.txt`). Sentry `karaman/qminesweeper` —
  `search_issues` for release `qminesweeper@1.17.0` (both
  `is:unresolved` and any-status) returned **zero issues**.
  Expected — opt-in telemetry, assets just published, zero installs
  have had a chance to fire a session yet. GitHub release body
  rewritten from the auto-generated stub to user-facing prose
  covering the palette switch, opt-in default, per-platform
  downloads, and the macOS quarantine note. Watch closed.
- **Next candidates:**
  - Save-and-resume games across launches (still parked — board-state
    + marker-state + timer-offset serialization).
  - Per-layout best-time leaderboard (mine-position hash + new
    persistence schema).
  - Hint button (limited per-game, exposes 1 safe cell at a cost)
    — needs a small deterministic solver.
  - Daily-streak / games-played-today mini-counter on the win
    dialog — derived from QSettings + QDate, still smaller than
    save-and-resume.
  - MM:SS timer format for games crossing 60 s — small polish,
    zero new translatable strings if the format is composed at
    runtime.

## 2026-04-25 — Cycle 13 — v1.16.0 (autonomous)

- **Chosen problem:** No win-streak tracking. The project had been
  building out the speedrun-aware story (best-time-with-date in
  v1.3.0, no-flag bracket in v1.13.0, 3BV/s in v1.14.0, efficiency
  in v1.15.0) but lacked the most basic engagement loop —
  consecutive-wins counting per difficulty. Last cycle's "Next
  candidates" list re-stated the same four parked-multiple-times
  items (save-and-resume, per-layout leaderboard, color-blind
  palette, hint button); win streak slots in underneath all of
  them as a small, additive Stats schema extension with zero risk
  to game logic.
- **Evidence:** No `streak`/`currentStreak`/`bestStreak` anywhere
  in tree. `Stats::Record` already exposed `played`/`won`/`bestSeconds`
  per difficulty in QSettings; adding a sibling pair was a one-file
  schema bump. Win-streak engagement mechanics ship in essentially
  every modern Minesweeper variant (Minesweeper Online, Microsoft
  Solitaire Collection, GNOME Mines).
- **Shipped:**
  - Branch: `feat/win-streak` (squash-merged + deleted)
  - PR: [#34](https://github.com/Mavrikant/QMineSweeper/pull/34)
  - Tag: `v1.16.0`
  - Release: [v1.16.0](https://github.com/Mavrikant/QMineSweeper/releases/tag/v1.16.0)
- **Diff shape:** 18 files, +896/-414 LOC — productive slice
  ~280 LOC (stats.h +30, stats.cpp +24, mainwindow.cpp +27,
  mainwindow.h +1, tests +186 with 11 new cases + a few existing
  test updates for the new return type, DECISIONS +54). The
  remaining churn is per-locale `.ts` line-rewriting that lupdate
  emits whenever a new source string is added, plus a 27-line
  `apply_translations.py` delta. Well under the 400-LOC cycle cap.
- **Translation cost:** 3 new format strings × 9 hand-translated
  locales = 27 fresh hand-translations. All 10 locales now 91/91
  finished, 0 unfinished.
- **Assumptions made:**
  - **Per-difficulty, not global.** A Beginner win after an Expert
    loss must not extend a "global" streak. Per-difficulty matches
    the rest of the schema.
  - **Lifetime, not time-windowed.** Matches `played`/`won`/
    `bestSeconds`'s semantics and avoids a clock-based invalidation
    path.
  - **Strict `>` for best-streak update.** Tying the previous best
    does not re-stamp the date — `bestStreakDate` is pinned on the
    moment the bar was *first* reached, then re-pinned only when
    actually surpassed. Tested explicitly with the 1→2→3→loss→1→2
    →3→4 sequence in `testBestStreakDateStampedOnHighWaterOnly`.
  - **Replays / Custom excluded.** Same exclusion rule as
    `played`/`won`/`bestSeconds` — neither a memorised-board win
    nor a custom-grid loss can game the streak.
  - **`New Game` mid-run does not count as a loss.** Same convention
    as `played` — only an explicit Lost transition resets.
  - **Streak ≥ 2 to show on the win dialog.** A single win already
    feels celebratory; a "Streak: 1" flair would be noise.
  - **`newBestStreak` swap, not stack.** When the win pushes the
    bar past the prior best, the dialog shows
    `🌟 New best streak: N!` *instead of* (not in addition to) the
    plain `🔥 Streak: N`. Avoids two near-duplicate lines on the
    same dialog.
  - **`explicit operator bool()` on `WinOutcome`.** Keeps the prior
    `recordWin(...) returns bool` semantics for `QVERIFY(...)` and
    `if (...)` while forcing existing assignment-to-bool sites to
    use `.newRecord` explicitly. Prevents silent truncation if a
    future caller stores the outcome.
  - **Telemetry additive.** `streak` (int) and `new_best_streak`
    (bool string) join the existing `game.won` tag set. No new
    event type.
- **Skipped:**
  - *Save-and-resume games across launches.* Still bigger
    (board-state + marker-state + timer-offset serialization +
    QSettings schema bump); parked across multiple cycles.
  - *Per-layout best-time leaderboard.* Would need a hash of mine
    positions + new persistence schema; bigger than this cycle.
  - *Color-blind-friendly cell number palette.* Real accessibility
    win, but defining a deuteranopia/protanopia-safe 8-colour
    palette is design work.
  - *Hint button (limited per-game, exposes 1 safe cell at a time
    cost).* Interesting risk-reward UX feature, requires a small
    deterministic solver.
  - *Streak resetting on `New Game` mid-run.* Matches `played`'s
    behaviour — only an explicit Lost transition counts.
  - *Streak shown for current = 1 wins.* Noise; engagement payoff
    starts at the second consecutive win.
  - *Live streak indicator on the main window.* Would need a fourth
    label slot in the toolbar; the Statistics dialog is the right
    place to look it up between games.
- **Risks logged:** none new. The Stats schema extension is
  additive and the `WinOutcome.operator bool()` is `explicit`, so
  no existing call site silently changes meaning. Legacy records
  with no `streak_*` keys load as `0 / 0 / invalid` —
  `testLegacyRecordWithoutStreakLoadsAsZero` pins this.
- **UI smoke:** Deferred. The cron-launched task context lacks
  display capture permissions. Logic is covered by 34 deterministic
  unit tests in `tst_stats` (11 new + 23 existing); the dialog
  format is a single `arg(int)` chain. Manual end-to-end verification
  will happen on the next interactive session if any visual
  regression appears.
- **Post-release watch (T+~3min):** Release workflow
  [run 24919628652](https://github.com/Mavrikant/QMineSweeper/actions/runs/24919628652)
  green across all three platforms; five assets published (Linux
  AppImage, Linux tar.gz, macOS universal DMG, Windows x64 ZIP,
  plus `SHA256SUMS.txt`). Sentry `karaman/qminesweeper` —
  `search_issues` for unresolved issues in release
  `qminesweeper@1.16.0` in the last hour returned **zero results**.
  Expected — telemetry is opt-in and assets were just published, no
  install has had a chance to fire a session yet. GitHub release
  body rewritten from the auto-generated stub to user-facing prose
  covering the streak mechanic, exclusions, the per-platform
  downloads, and the macOS quarantine note. Watch closed.
- **Next candidates:**
  - Save-and-resume games across launches (still parked — would
    need board-state + marker-state + timer-offset serialization).
  - Per-layout best-time leaderboard (would need a hash of mine
    positions + new persistence schema).
  - Color-blind-friendly cell number palette (real accessibility
    value, design-level work).
  - Hint button (limited per-game, exposes 1 safe cell at a time
    cost) — interesting risk-reward UX feature, requires a small
    deterministic solver.
  - Daily-streak / "games played today" mini-counter on the win
    dialog — derived from QSettings + QDate, smaller than save-
    and-resume.

## 2026-04-25 — Cycle 12 — v1.15.0 (autonomous)

- **Chosen problem:** No click count or efficiency % on the win dialog.
  The project has shipped the speedrun trio's first two members —
  3BV alone in v1.14.0 and 3BV/s as the per-second rate — but the
  third canonical metric, `Efficiency = 3BV / clicks · 100`, was
  parked last cycle pending a click counter. With the 3BV/s rhythm
  established and the userClick gesture-vs-cascade distinction
  cleanly expressible via a new MineButton signal, the click counter
  is no longer a "for marginal payoff" parking call — it ships now
  and lands the speedrun-completeness story.
- **Evidence:** No `userClicks` / `clickCount` / `m_clicks` anywhere
  in tree. `MainWindow::onGameWon` already had `boardValue()` from
  v1.14.0 — only the divisor was missing. Speedrun communities (MS
  Online, MS Arbiter, Active Minesweeper) report uncapped efficiency
  alongside time/3BV/3BV/s as the four-number summary.
- **Shipped:**
  - Branch: `feat/click-count-efficiency` (squash-merged + deleted)
  - PR: [#33](https://github.com/Mavrikant/QMineSweeper/pull/33)
  - Tag: `v1.15.0`
  - Release: [v1.15.0](https://github.com/Mavrikant/QMineSweeper/releases/tag/v1.15.0)
- **Diff shape:** 20 files, +1072/-670 LOC — productive slice
  ~250 LOC (MineField counter + reset 5, MineButton signal 2,
  mainwindow dialog 8 + telemetry 4, header churn 22, tests
  +209 with 15 new cases + 1 deflake). The remaining churn is
  per-locale `.ts` line-rewriting that lupdate emits whenever a new
  source string is added, plus a 9-line `apply_translations.py`
  delta. Well under the 400-LOC cycle cap.
- **Translation cost:** 1 new format string × 9 hand-translated
  locales. Trailing `%` kept as a literal across all locales for
  consistency and to dodge Qt's `%%`-vs-`%n` parser ambiguity. Arabic
  uses U+066A (٪) for the percent character, matching native
  typography. All 10 locales now 88/88 finished, 0 unfinished.
- **Assumptions made:**
  - **Useful click = gesture that opened ≥1 cell.** Defines the
    denominator. Right-click flag toggles, no-op left-clicks
    (already-opened or flagged cells), and unsatisfied chords
    (`flagsAround != cell->Number()`) are excluded. A satisfied
    chord whose unflagged neighbours are all already opened is also
    a no-op — `revealedAny == false` and the counter stays put.
  - **Chord that hits a wrong-flag mine still counts.** It opened
    a cell (the mine). The run ends in a loss but the gesture was a
    useful click; the test
    `testUserClicksChordWrongFlagCountsAndExplodes` pins this. Only
    matters in telemetry — the loss dialog never shows efficiency.
  - **Flood-fill cascade is one click, not N.** The whole point of
    the metric. Implemented by emitting `MineButton::userClick` only
    from `mousePressEvent`, never from `Open()` itself; flood-driven
    `Open()` calls in `onCheckNeighbours` and chord-neighbour loops
    don't go through the gesture entry, so they never trigger the
    increment.
  - **Keyboard parity inline.** `MineField::handleCellKey` increments
    `m_userClicks` directly for Space/Enter on unopened non-flagged
    cells (mirrors the mouse path's pre-Open guard). Space/Enter/D
    on opened cells route through `onChordRequested`, which is the
    chord counting site — no double-count.
  - **Uncapped efficiency.** Chord-heavy play legitimately yields
    >100 %; capping discards information. The dialog displays the
    raw rounded value (`std::lround(100.0 * bv / clicks)`).
  - **No live ticker, no Stats column.** Same trade as 3BV/s in
    cycle 11 — display-only on the win dialog preserves the metric
    without inflating the per-difficulty bracket schema or
    distracting the player during a run.
  - **Suppressed on `clicks == 0`.** Defensive for fixed-layout test
    setups that reach `onGameWon` with no user gesture; mirrors the
    `bv == 0` guard.
  - **Telemetry additive.** `clicks` (int) and `efficiency` (int %)
    join the existing `bv` / `bv_per_second` / `noflag` /
    `new_record` / `replay` tags on the `game.won` event. No new
    event type.
- **Drive-by:** `testAnyFlagPlacedResetByReplay` was flaking about
  50 % of runs (3/5 fails on `git stash` + 5 reruns of main). The
  test relied on a random Beginner first-click producing a layout
  where `(0, 0)` is unopened, but a flood from `(4, 4)` could reach
  `(0, 0)` and turn the test's `cycleMarker` into a no-op. Switched
  to a deterministic `setFixedLayout(5, 5, {{0, 0}})` and a `(2, 2)`
  flag cell. Verified across 8 consecutive runs post-fix. Pinned
  along with the new feature in #33 to keep CI green for the
  release.
- **Skipped:**
  - *Save-and-resume games across launches.* Still bigger
    (board-state + marker-state + timer-offset serialization +
    QSettings schema bump); parked across multiple cycles.
  - *Per-layout best-time leaderboard.* Would need a hash of the
    mine positions + new persistence schema; bigger than this cycle.
  - *Color-blind-friendly cell number palette.* Real accessibility
    win, but defining a deuteranopia/protanopia-safe 8-colour
    palette is more design work than a one-cycle increment can
    absorb cleanly.
  - *Click count + efficiency in Stats.* Captured under "Assumptions
    made" — the schema cost was the dominant reason.
  - *Efficiency on the loss dialog.* A losing board never reaches
    full clear, so the metric is meaningless before the run ended;
    consistent with 3BV/s also being skipped on losses.
- **Risks logged:** none new. The new signal-based counting path is
  paranoidly tested (15 cases including flood-counts-once,
  flagged-no-op, opened-no-op, right-click-no-op, satisfied-chord-no-op,
  wrong-flag-chord-counts, plus reset-by-newGame / replay /
  setFixedLayout). The Open()-vs-gesture cut is the load-bearing
  invariant and tested by `testUserClicksFloodCountsOnce` —
  guarantees the counter doesn't drift if anyone refactors the flood
  path later.
- **UI smoke:** Deferred. The cron-launched task context lacks
  display capture permissions (`screencapture -l`/`-R` both errored
  with "could not create image from window/rect"). Logic is covered
  by 15 deterministic unit tests; the dialog format is a single
  `arg(int).arg(int)` chain. Manual end-to-end verification will
  happen on the next interactive session if any visual regression
  appears.
- **Post-release watch (T+~3min):** Release workflow
  [run 24918547649](https://github.com/Mavrikant/QMineSweeper/actions/runs/24918547649)
  green across all three platforms in ~2m07s; five assets published
  (Linux AppImage, Linux tar.gz, macOS universal DMG, Windows x64
  ZIP, plus `SHA256SUMS.txt`). Sentry `karaman/qminesweeper` —
  `search_issues` for unresolved issues in release
  `qminesweeper@1.15.0` in the last hour returned **zero results**.
  Expected — telemetry is opt-in and assets were just published, no
  install has had a chance to fire a session yet. GitHub release
  body rewritten from the auto-generated stub to user-facing prose
  covering the metric definition, the dialog format, the per-platform
  downloads, and the macOS quarantine note. Watch closed.
- **Next candidates:**
  - Save-and-resume games across launches (still parked — would
    need board-state + marker-state + timer-offset serialization).
  - Per-layout best-time leaderboard (would need a hash of mine
    positions + new persistence schema).
  - Color-blind-friendly cell number palette (real accessibility
    value, design-level work).
  - Hint button (limited per-game, exposes 1 safe cell at a time
    cost) — interesting risk-reward UX feature, requires a small
    deterministic solver.

## 2026-04-25 — Cycle 11 — v1.14.0 (autonomous)

- **Chosen problem:** No efficiency metric on the win dialog. The
  project has been leaning speedrun-aware for several cycles
  (best-time-with-date in 1.3.0, no-flag bracket in 1.13.0) but it
  still didn't surface 3BV — the canonical Minesweeper community
  measure of how hard the board was — or 3BV/s, the per-second
  efficiency rate that tournaments worldwide use as the single-number
  skill measure. Natural follow-on to v1.13.0; display-only, no Stats
  schema break, no risk to game logic.
- **Evidence:** No `compute3BV`/`boardValue`/`bv` anywhere in tree.
  `MainWindow::onGameWon` already had `m_lastElapsedSeconds`; the only
  missing piece was a board-side number to divide by. Standard in
  Minesweeper Online, Minesweeper Arbiter, Active Minesweeper.
- **Shipped:**
  - Branch: `feat/3bv-metric` (squash-merged + deleted)
  - PR: [#32](https://github.com/Mavrikant/QMineSweeper/pull/32)
  - Tag: `v1.14.0`
  - Release: [v1.14.0](https://github.com/Mavrikant/QMineSweeper/releases/tag/v1.14.0)
- **Diff shape:** 18 files, +763/-375 LOC — ~108 of which is
  `MineField::compute3BV` + integration (4 wire-points: newGame reset,
  onCellPressed, newGameReplay, setFixedLayout), 12 in the header,
  ~20 in `MainWindow` (dialog format + telemetry tags), 103 in
  `tests/tst_minefield.cpp` (9 new cases), 92 in `DECISIONS.md`,
  9 in `apply_translations.py`, and the rest is `.ts` regeneration
  across 10 locales. Productive slice (C++ + tests) ~245 LOC, well
  under the 400-LOC cycle cap.
- **Translation cost:** 1 new format string × 9 non-English locales
  = 9 hand translations. "3BV" is invariant in the speedrun
  community; the "/s" abbreviation is locale-aware where conventions
  exist (RU `/с`, ZH `/秒`, AR `/ث`, TR `/sn`, FR spacing rules) —
  others stay `/s` per international convention. All 10 locales now
  87/87 finished, 0 unfinished.
- **Assumptions made:**
  - **Two-pass BFS algorithm.** Pass 1 flood-fills each connected
    zero-region through the 8-neighborhood (`std::queue` with a
    visited bitmap), counting one BV per region and marking the
    numbered fringe cells visited along the way. Pass 2 sweeps for
    every remaining unvisited non-mine cell — those are isolated
    numbered cells not adjacent to any zero, and each requires its
    own click. Mines are never counted. Result matches the
    speedrun-community canonical 3BV definition.
  - **Computed once at mine placement, cached.** `m_boardValue` is
    set in three places — `onCellPressed` (after `fillMines` +
    `fillNumbers`), `newGameReplay` (after re-applying mines +
    `fillNumbers`), and `setFixedLayout` (same). Reset to 0 in
    `newGame()` so the cache cannot leak from a prior game. Pure
    `const` read after that.
  - **3BV/s div-by-zero guard at 0.05s.** Real play always crosses
    the 0.1s timer-tick before `onGameWon` fires; the floor is for
    the test pathway through `setFixedLayout` where elapsed time
    can be exactly zero.
  - **Replay and Custom wins get the metric too.** It's a property
    of the *run*, not a leaderboard claim. Excluding them would
    make the dialog feel inconsistent for what's view-only data;
    Replay-vs-Replay 3BV/s comparisons on the same layout are
    genuinely useful for self-coaching.
  - **No Stats column.** Already 5 columns after v1.13.0; adding a
    6th (and its `_date` companion) for a per-run-shape number
    would inflate the bracket count without value. Display-only on
    the dialog preserves the metric without the schema cost.
  - **No live ticker during play.** Speedrunners measure
    final-time-rate, not running-rate; a live `displayTimer`-driven
    update would distract more than inform.
  - **Telemetry additive.** `bv` (int) and `bv_per_second` (float,
    2dp) join the existing `noflag`/`new_record`/`replay` tags on
    the `game.won` event. No new event type.
  - **Suppressed on `bv == 0`.** Defensive: a hand-rolled
    `setFixedLayout` win during a test mid-sweep can technically
    reach `onGameWon` with bv = 0; the dialog skips the line in
    that case rather than printing "3BV: 0 · 3BV/s: 0.00".
- **Skipped:**
  - *Save-and-resume games across launches.* Still bigger
    (board-state + marker-state + timer-offset serialization +
    QSettings schema bump); parked for a future cycle.
  - *Overlay-with-bubbles tutorial upgrade.* Cosmetic; no
    complaints since v1.10.0.
  - *Undo last action.* Risky — interacts with first-click safety
    and mine placement timing; also undermines the no-flag bracket
    semantics. Park.
  - *Efficiency % (3BV / clicks).* Would need a click counter on
    `MineField` for marginal payoff over 3BV/s. Park for a
    follow-on cycle if a click-count is wanted for other reasons.
  - *3BV in Stats column.* Captured under "Assumptions made" — the
    schema cost was the dominant reason.
- **Risks logged:** none new. Worst-case Expert compute is 480 cells
  × 9-neighborhood BFS = ~4320 visit ops, runs once at game start in
  microseconds — measured no detectable runtime delta in the existing
  `tst_minefield` 0.73s budget.
- **Post-release watch (T+~3min):** Release workflow
  [run 24917034512](https://github.com/Mavrikant/QMineSweeper/actions/runs/24917034512)
  green across all three platforms in ~1m42s; five assets published
  (Linux AppImage 35.9 MB, Linux tar.gz 35.5 MB, macOS universal DMG
  23.2 MB, Windows x64 ZIP 44.1 MB, plus `SHA256SUMS.txt`). Sentry
  `karaman/qminesweeper` — `search_issues` for unresolved issues in
  release `qminesweeper@1.14.0` in the last hour returned **zero
  results**. Expected — telemetry is opt-in and assets were just
  published, no install has had a chance to fire a session yet.
  GitHub release body rewritten from the auto-generated stub to
  user-facing prose covering the metric definition, the dialog
  format, per-platform downloads, and the macOS quarantine note.
  Watch closed.
- **Next candidates:**
  - Save-and-resume games across launches (parked across multiple
    cycles — would need board-state + marker-state + timer-offset
    serialization).
  - Efficiency % via click-count (small follow-on if 3BV/s sees
    user interest).
  - Per-layout best-time leaderboard (would need a hash of the
    mine positions + a new persistence schema; bigger).
  - Overlay-with-bubbles tutorial upgrade (still no complaints).

## 2026-04-25 — Cycle 10 — v1.12.0 (autonomous)

- **Chosen problem:** Timed runs could be interrupted mid-game (phone
  call, doorbell, Slack ping) and the player had no way to stop the
  clock. Best-time counts drifted past truth or the user abandoned a
  winning Expert run. Pause/resume has been on the "Next candidates"
  list for **seven consecutive cycles** (#3–#9) as the highest-value
  deferred feature; every prior cycle either rejected it as
  timer/state-machine risk or had a smaller contained pick. At cycle
  10, the risk profile is finally contained (the keyboard-nav
  eventFilter from v1.11.0 gave us a natural pre-existing seam) and
  no smaller item dominates — this was the cycle to take it.
- **Evidence:** No `isPaused`/`setPaused` anywhere in the tree.
  `QElapsedTimer` used directly in `MainWindow::elapsedSeconds()` with
  no offset accumulator — so if the timer were to resume after a
  pause, it would reflect wall-clock time not playing time. Standard
  in GNOME Mines; Windows Minesweeper Classic shipped pause via the
  menu too.
- **Shipped:**
  - Branch: `feat/pause-resume` (squash-merged + deleted)
  - PR: [#30](https://github.com/Mavrikant/QMineSweeper/pull/30)
  - Tag: `v1.12.0`
  - Release: [v1.12.0](https://github.com/Mavrikant/QMineSweeper/releases/tag/v1.12.0)
- **Diff shape:** 18 files, +1211/-552 LOC — ~197 of which is the
  real C++ diff (`MineField::setPaused` + overlay + eventFilter guard
  79, `MainWindow` pause action + offset accumulator + clearPauseState
  97, headers 19, CMakeLists bump 2), 153 in `tests/tst_minefield.cpp`
  (9 new cases), 96 in `DECISIONS.md`, 27 in
  `apply_translations.py`, and the rest is `.ts` regeneration across
  10 locales. Productive slice (C++ + tests) ~350 LOC, under the
  400-LOC cycle cap.
- **Translation cost:** 3 new strings (`&Pause`, `&Resume`, `Paused`)
  × 9 non-English locales = 27 hand translations. All 10 locales now
  84/84 finished, 0 unfinished.
- **Assumptions made:**
  - **No new `GameState` enum value.** Pause is an orthogonal
    cross-cutting freeze (input + timer), not a logical state. Parallel
    `m_paused` booleans in `MineField` and `MainWindow` keep the
    existing state machine, `checkWin`, `onChordRequested`, and every
    test asserting against state untouched. Adding `Paused` to the
    enum would have rippled through ~30 tests.
  - **Block input via `MineField::eventFilter`, not `setCellEnabled`.**
    The enabled flag is the win/loss freeze path; pause is temporary,
    so an eventFilter guard (swallows MouseButtonPress / Release /
    DblClick / KeyPress / KeyRelease — but **not**
    `ShortcutOverride`) keeps cells visually unchanged so the user
    sees their board exactly as it was. Letting ShortcutOverride pass
    means P / Ctrl+Q / F2 still work from the paused state.
  - **Overlay is a `QFrame` child of `MineField`.** Translucent
    `rgba(0,0,0,140)` background with a centered "Paused" label,
    geometry locked to `rect()`, `raise()` on show. Absorbs mouse
    events by z-order so the eventFilter is defence-in-depth for the
    keyboard/focus paths the overlay can't catch.
  - **Timer offset via `m_pausedTotalMs` + `m_pauseStartMs`.**
    `QElapsedTimer` has no pause API. On pause, snapshot
    `m_pauseStartMs = m_gameTimer.elapsed()`; on resume, add
    `(elapsed() - m_pauseStartMs)` to `m_pausedTotalMs`.
    `elapsedSeconds()` returns `(raw - paused) / 1000`. Snapshotting
    `m_lastElapsedSeconds = elapsedSeconds()` at the moment of pause
    makes the `displayTimer`-stopped label freeze on the last
    playing-seconds reading, not drift to zero.
  - **`P` shortcut at `WindowShortcut` context.** QAction default.
    Fires before focused-widget keyPress, so a focused cell doesn't
    need to know about the shortcut and the eventFilter doesn't need
    a `Key_P` case. Matches Windows Minesweeper Classic + GNOME Mines
    convention.
  - **Pause auto-clears on every state transition.** `newGame`,
    `newGameReplay`, difficulty change, win, and loss all call
    `clearPauseState()` before anything else. Eliminates the "I
    changed difficulty while paused and now the new game starts
    paused" failure mode.
  - **No persistence.** Pause is in-session only. Saving paused-at-X
    to QSettings would imply game-state persistence across launches
    (a much bigger "save and resume" feature). If the user quits
    while paused, the game is lost — documented in the PR.
  - **Breadcrumb-only telemetry.** Adding a `game.paused` event type
    would balloon the Sentry quota for a low-signal lifecycle hook.
    The `game.won` / `game.lost` events already use post-offset
    `elapsedSeconds()`, so a paused-then-resumed Expert run records
    its true playing time, not wall-clock time.
- **Skipped:**
  - *No-flag speedrun achievement.* Parked for the second cycle —
    small (1 string × 9 locales, ~50 LOC) but strictly dominated by
    pause/resume on user-visible value this cycle.
  - *Overlay-with-bubbles tutorial upgrade.* Cosmetic; still no
    complaints since v1.10.0.
  - *Save-and-resume games across launches.* New idea surfaced when
    deciding pause wouldn't persist — genuinely bigger feature
    (serialize board state + revealed/flagged/question per cell +
    timer offset), parked.
- **Risks logged:** The `QElapsedTimer` offset accumulator is subject
  to integer wrap on 32-bit platforms after ~24 days of continuous
  play — not a realistic risk for this app (game sessions are under
  an hour), but explicit `qint64` types ensure 64-bit safety on every
  platform. The translucent overlay's stylesheet uses `rgba(0,0,0,140)`
  which relies on Qt's alpha-blending support; confirmed working on
  all three platforms via CI smoke builds plus local macOS run.
- **Post-release watch (T+~3min):** Release workflow
  [run 24915294501](https://github.com/Mavrikant/QMineSweeper/actions/runs/24915294501)
  green across all three platforms in ~2 min (build-cache hit on all
  runners); five assets published (Linux AppImage, Linux tar.gz,
  macOS universal DMG, Windows x64 ZIP, `SHA256SUMS.txt`). Sentry
  `karaman/qminesweeper` — `search_issues` for unresolved issues in
  release `qminesweeper@1.12.0` in the last hour returned **zero
  results**. Expected — telemetry is opt-in and assets were just
  published, no install has had a chance to fire a session yet.
  GitHub release body rewritten from the auto-generated stub to
  user-facing prose covering the overlay, input blocking, timer
  freeze, auto-clear semantics, per-platform downloads, and the
  macOS quarantine note. Watch closed.
- **Next candidates:**
  - No-flag speedrun achievement (second cycle parked, ripe for a
    small contained pick).
  - Save-and-resume games across launches (bigger — would need
    board-state + marker-state + timer-offset serialization and a
    QSettings schema bump).
  - Overlay-with-bubbles tutorial upgrade (optional, still no
    complaints).
  - Undo last action (risky — would interact with first-click safety
    and mine placement timing).

## 2026-04-24 — Cycle 9 — v1.11.0 (autonomous)

- **Chosen problem:** The minefield was mouse-only. `MainWindow` wired
  menu shortcuts (F2 / Ctrl+Q / etc.) but focusing a cell with Tab did
  nothing useful — arrow keys didn't move focus, Space/Enter did
  nothing on closed cells (Qt's default `QAbstractButton` handling was
  wrong for a game grid), and there was no visible focus indicator.
  Keyboard navigation has been on the "Next candidates" list for five
  consecutive cycles (#7–#8) and is the baseline accessibility ask for
  a grid-based game; this cycle took it.
- **Evidence:** No `keyPressEvent` or `installEventFilter` anywhere in
  `MineButton` or `MineField`. `setFocusPolicy` unset (macOS
  defaulted `QPushButton` to `Qt::TabFocus`, blocking click-focus).
  No focus-ring paint code. `MineButton` had no public
  `row()` / `col()` getters — the grid coords were only stored as
  private members.
- **Shipped:**
  - Branch: `feat/keyboard-navigation` (squash-merged + deleted)
  - PR: [#29](https://github.com/Mavrikant/QMineSweeper/pull/29)
  - Tag: `v1.11.0`
  - Release: [v1.11.0](https://github.com/Mavrikant/QMineSweeper/releases/tag/v1.11.0)
- **Diff shape:** 7 files, +427/-2 LOC — 111 of which is the
  `MineField::eventFilter` / `handleCellKey` / `focusCell` path,
  ~35 in `MineButton` (row/col getters, `Qt::StrongFocus`, focus-ring
  `paintEvent`), 163 in `tests/tst_minefield.cpp` (8 new cases),
  107 in `DECISIONS.md`. Real-code slice well under the 400-LOC cycle
  cap. No translation churn.
- **Translation cost:** 0 new strings. All 10 locales untouched —
  still 81/81 finished, 0 unfinished.
- **Assumptions made:**
  - **Event filter on `MineField`, not per-cell `keyPressEvent`.**
    `MineField` installs itself as event filter on every `MineButton`
    so it intercepts arrow/Space/Enter before `QAbstractButton`'s
    default keyPress eats them. Centralising keeps `MineButton`
    back-pointer-free (architecture invariant preserved).
  - **`Qt::StrongFocus` set explicitly.** macOS `QPushButton` defaults
    to `Qt::TabFocus`, which blocks click-to-focus. Must be set so
    a mouse-click on a cell immediately activates keyboard nav.
  - **Focus ring drawn in `paintEvent`, not `:focus` stylesheet.**
    The cell's stylesheet changes as it opens / gets flagged /
    explodes — a `:focus` pseudo-state would fight those. Painting in
    `paintEvent` is stylesheet-independent and survives every state
    transition.
  - **Space/Enter dispatches on opened-state.** On a covered cell it
    opens; on an opened number it chords (matching middle-click). D
    is the dedicated force-chord for users who prefer Space = reveal
    only.
  - **`MineButton::cycleMarker()` public.** The F keybind drives it
    directly from `MineField::handleCellKey` — making it public is
    cheaper than a second `QMetaObject::invokeMethod`-via-signals
    hop and matches the "signals up, slots down" invariant (the
    method is a slot-equivalent, not a signal).
  - **Only arrows active after Won/Lost.** The board is frozen by
    `freezeAllCells`; allowing F/Space/Enter post-game would either
    be a no-op or confuse state. Arrows still work so the user can
    inspect the revealed layout.
  - **No auto-focus on `gameStarted`.** The user takes focus by
    clicking a cell. Auto-focusing a cell on load would fight tab
    order for users driving menus from the keyboard first.
- **Skipped:**
  - *Pause / resume.* Parked now for the seventh cycle running; still
    the highest-value deferred candidate.
  - *No-flag speedrun achievement.* Parked one more cycle; budget
    spent on the accessibility path.
  - *Overlay-with-bubbles tutorial upgrade.* Parked, no complaints
    since v1.10.0 shipped.
  - *Custom difficulty (dialog-driven width/height/mines).* New idea
    surfaced during keyboard-nav review — parked for a future cycle.
- **Risks logged:** Platform drift on `Qt::TabFocus` vs.
  `Qt::StrongFocus`. Explicit `setFocusPolicy(Qt::StrongFocus)` in
  `MineButton` ctor neutralises the Mac default; Linux/Windows were
  already `StrongFocus` by default so no regression. If a future Qt
  bump changes `QPushButton`'s default, the explicit call still wins.
- **Post-release watch (T+~5min):** Release workflow
  [run 24901736532](https://github.com/Mavrikant/QMineSweeper/actions/runs/24901736532)
  green across all three platforms in ~2 min (build-cache hit on all
  runners); five assets published (Linux AppImage 35.9 MB, Linux
  tar.gz 35.5 MB, macOS universal DMG 23.2 MB, Windows x64 ZIP
  44.1 MB, plus `SHA256SUMS.txt`). Sentry `karaman/qminesweeper` —
  `search_issues` for unresolved issues in release
  `qminesweeper@1.11.0` in the last hour returned **zero results**.
  Expected — telemetry is opt-in and assets were just published, no
  install has had a chance to fire a session yet. Pre-existing
  `qminesweeper@1.10.0` traffic shows the three benign game-lifecycle
  events (`game.started` / `game.won` / `game.lost`) grouped as
  issues — these are intentional telemetry breadcrumbs, not crashes.
  GitHub release body rewritten from the auto-generated stub to
  user-facing prose covering the four keybinds, the Won/Lost arrow
  gating, per-platform downloads, and the macOS quarantine note.
  Watch closed.
- **Next candidates:**
  - Pause / resume (P shortcut) with board-covering overlay.
  - No-flag speedrun achievement.
  - Custom difficulty dialog (width × height × mine-count inputs).
  - Overlay-with-bubbles tutorial upgrade (optional, only if
    complaints arrive).

## 2026-04-24 — Cycle 8 — v1.10.0 (closes #26)

- **Chosen problem:** First-time players landed on the minefield cold —
  no explanation of left-click / right-click / chord / numbers / menus.
  @Mavrikant filed issue
  [#26](https://github.com/Mavrikant/QMineSweeper/issues/26) asking for
  a tutorial that auto-launches on first run and is re-openable from
  the menu. The prior cycle's Next candidates still point at
  pause/resume and keyboard-nav; this cycle took the explicit GitHub
  ask instead.
- **Evidence:** No `Help → Tutorial` anywhere in
  [mainwindow.cpp](mainwindow.cpp); no QSettings flag for "has been
  told how to play"; `Help → About` was the only existing Help entry.
- **Shipped:**
  - Branch: `feat/first-run-tutorial` (squash-merged + deleted)
  - PR: [#28](https://github.com/Mavrikant/QMineSweeper/pull/28)
  - Tag: `v1.10.0`
  - Release: [v1.10.0](https://github.com/Mavrikant/QMineSweeper/releases/tag/v1.10.0)
  - Closes: #26
- **Diff shape:** 15 files, roughly +210 LOC real code
  (100 tutorial module + 80 tests + ~30 MainWindow glue) + the usual
  translation churn. Under the 400-LOC cycle cap.
- **Translation cost:** 19 new hand-translated strings × 9 non-English
  locales (171 new dict entries). Every locale reports 81/81 finished,
  0 unfinished — coverage preserved.
- **Assumptions made:**
  - **Sequential modal card, not pointing-bubble overlay.** Dialog
    with Back / Next / Skip + `Step N of M`. Cheaper (~150 LOC vs.
    ~400), consistent with the existing Stats / consent dialogs, and
    upgradable to an overlay in a future cycle if anyone asks.
  - **Skip marks completed.** The whole point is not to re-prompt on
    every launch once the user has declined — the Help menu is always
    available to re-open. Any more nuance (e.g. "remind me next time")
    would need a third state for a single-bit user decision.
  - **Deferred first show via `QTimer::singleShot(0, …)`** so the
    main window paints before the dialog lifts, matching how the
    consent prompt already behaves.
  - **Tutorial fires after (not alongside) the consent dialog** —
    `exec()` is modal and blocking, so the consent `QMessageBox`
    finishes first, then the tutorial appears.
  - **Existing 1.9.0 installs count as "not completed"** (the key is
    absent in their plist). That matches #26's "automatically launches
    upon the initial execution" — first 1.10.0 launch.
- **Skipped:**
  - *Pause / resume.* Parked now for the sixth cycle running; still the
    highest-value deferred candidate when a human-directed cycle is
    willing to absorb the timer/state-machine risk.
  - *Keyboard navigation.* Parked for the fifth cycle running; worth
    its own dedicated cycle for the focus-management refactor.
  - *No-flag speedrun achievement.* Parked one more cycle; budget used.
  - *Overlay-with-bubbles upgrade* of the tutorial. Sequential card
    ships the value; a future cycle can revisit shape if Sentry or a
    user surfaces an actual complaint.
- **Risks logged:** none new. Emoji-font concerns from v1.8.0/1.9.0
  don't apply — tutorial uses plain Unicode only.
- **Post-release watch (T+~5min):** Release workflow
  [run 24900943307](https://github.com/Mavrikant/QMineSweeper/actions/runs/24900943307)
  green across all three platforms; five assets published (Linux
  AppImage 34.2 MB, Linux tar.gz 33.9 MB, macOS universal DMG 22.1 MB,
  Windows x64 ZIP 42.0 MB, plus `SHA256SUMS.txt`). Sentry
  `karaman/qminesweeper` — `search_issues` for unresolved issues in
  release `qminesweeper@1.10.0` in the last hour returned **zero
  results**. Expected — assets were just published, telemetry is
  opt-in, and no install has had a realistic chance to fire a session
  yet. GitHub release body rewritten from the auto-generated stub
  (222 → 2522 chars) to user-facing prose covering the six steps,
  menu re-opener, downloads per-platform, and the macOS quarantine
  note. Issue [#26](https://github.com/Mavrikant/QMineSweeper/issues/26)
  auto-closed by the squash-merge commit message (`state_reason=completed`
  at 2026-04-24T16:42:39Z). Watch closed.
- **Next candidates:**
  - Pause / resume (P shortcut) with board-covering overlay.
  - Keyboard navigation (arrow keys + space/F) for accessibility.
  - No-flag speedrun achievement.
  - Overlay-with-bubbles tutorial upgrade (optional, only if complaints
    arrive).

## 2026-04-24 — Cycle 7 — v1.9.0 (autonomous)

- **Chosen problem:** No tension smiley. v1.8.0 shipped the three static
  faces (🙂/😎/😵) but the header never reacted to an in-progress click —
  the classic Minesweeper "holding my breath" 😮 face that Windows
  Minesweeper, Minesweeper Arbiter, Minesweeper X and GNOME Mines all show
  while a cell is held down. Cycle 6's "Next candidates" listed this
  explicitly as follow-on polish; it was the smallest remaining gap vs.
  the reference clones.
- **Evidence:** `mainwindow.cpp` set the smiley from three signals
  (`gameStarted`, `gameWon`, `gameLost`) — no hook for mouse-down state.
  `MineButton::mousePressEvent` already existed; there was no
  `mouseReleaseEvent` override and no press/release-level signal. The
  plumbing was one `mouseReleaseEvent` away.
- **Shipped:**
  - Branch: `feat/tension-smiley` (squash-merged + deleted)
  - PR: https://github.com/Mavrikant/QMineSweeper/pull/27 (merged as `23eb0e7`)
  - Tag: `v1.9.0`
  - Release: https://github.com/Mavrikant/QMineSweeper/releases/tag/v1.9.0
  - CI: ubuntu + macos + windows + coverage + formatter green (Codacy
    `fail` — advisory, historically not blocking on this repo; same
    pattern as cycles 3–6). Combined status success.
- **Diff shape:** 10 files, +209/-2. Real code (excluding tests and
  `DECISIONS.md`): ~80 LOC across `MineButton` / `MineField` / `smiley.h` /
  `MainWindow`. Tests: ~125 LOC (5 new MineButton cases, 3 new smiley
  cases incl. full press→release integration through a real MineField).
  Well under the 400-LOC cycle cap.
- **Translation cost:** **Zero** new hand-translated strings. 😮 is a
  Unicode glyph on the same font fallback stack the already-shipped
  🙂/😎/😵 use. 58/58 finished preserved per locale. Second cycle in a row
  to ship user-visible UI with no translation delta.
- **Assumptions made:**
  - 😮 renders consistently via Apple Color Emoji / Segoe UI Emoji / Noto
    Color Emoji. Same stack as v1.8.0's faces; no rendering complaints in
    Sentry or GitHub issues since v1.2.0's 🏆.
  - Right-click-only presses deliberately skip tension — flag cycling is
    "mark-and-move", and flicking the header on every flag would churn.
  - `smileyForTensionState` keeps Won/Lost authoritative over tension so a
    late release on a frozen board can't strand 😮 on the header.
    Additionally, `setSmileyState` resets the tension flag on every game
    transition — belt-and-suspenders.
  - Cell-agnostic press signals on purpose — the header indicator doesn't
    care *which* cell is held, only *that* one is.
- **Skipped:**
  - *Pause / resume.* Still the highest-value parked candidate; bigger
    surface (overlay, timer arithmetic, ~3 new strings × 10 locales) and
    higher regression risk. Park for an 8th cycle.
  - *Keyboard navigation.* Medium surface, zero translation cost, good
    accessibility win. Reasonable next pick.
  - *Telemetry event for tension presses.* Would bloat the metric without
    any product question it answers; the existing game-start/won/lost
    events already carry the session signal.
- **Risks logged:** none new.
- **Post-release watch (T+~5min):** Release workflow
  [run 24899674160](https://github.com/Mavrikant/QMineSweeper/actions/runs/24899674160)
  green across all three platforms; five assets published
  (Linux AppImage 35.8 MB, Linux tar.gz 35.5 MB, macOS universal DMG
  23.1 MB, Windows x64 ZIP 44.1 MB, plus `SHA256SUMS.txt`). Sentry
  `karaman/qminesweeper` — `search_issues` for unresolved issues in
  release `qminesweeper@1.9.0` in the last hour returned **zero
  results**. Expected — assets were just published, telemetry is opt-in,
  no install has had a realistic chance to fire a session yet. The
  signal worth watching for is *any* new group tagged with the 1.9.0
  release; none observed. GitHub release body rewritten from the
  auto-generated template to user-facing prose covering behaviour,
  downloads per-platform, and the macOS quarantine note. Watch closed.
- **Next candidates:**
  - Pause / resume (P shortcut) with a board-covering overlay.
  - Keyboard navigation (arrow keys + space/F) for accessibility.
  - "Flag auto-correct hint" on new-record wins: a subtle indicator when
    the final win was achieved without using any flags (no-flag speedrun
    achievement).

## 2026-04-24 — Cycle 6 — v1.8.0 (autonomous)

- **Chosen problem:** No smiley-face status indicator. Every mainstream
  Minesweeper clone (Windows Minesweeper, Minesweeper Arbiter, GNOME
  Mines, the ones speedrun tournaments are played on) ships a 🙂/😎/😵
  button above the grid that doubles as a one-click new-game shortcut.
  Cycle 5's "Next candidates" list called it out explicitly as "iconic
  Minesweeper UX that ours lacks." Small surface, high nostalgia payoff,
  zero translation cost — an ideal fit for a single-cycle budget.
- **Evidence:** `mainwindow.ui` had only `mineCount` + `Time` in the top
  HBox with stretch `"1,1"`. No smiley widget, no click-to-reset path
  outside `Game → New`. The `GameState` enum + `gameStarted`/`gameWon`/
  `gameLost` signals were already emitted at every transition — the
  wiring existed, only a reactor was missing.
- **Shipped:**
  - Branch: `feat/smiley-indicator` (merged + deleted)
  - PR: https://github.com/Mavrikant/QMineSweeper/pull/25 (squash-merged as `57070bf`)
  - Release: https://github.com/Mavrikant/QMineSweeper/releases/tag/v1.8.0
  - Release workflow: [run 24866404642](https://github.com/Mavrikant/QMineSweeper/actions/runs/24866404642)
    — all three platforms green in ~2m07s (00:48:15Z → 00:50:22Z UTC).
    Five assets published: Linux AppImage + tar.gz, macOS universal DMG,
    Windows x64 ZIP, SHA256SUMS.txt.
  - CI: ubuntu + macos + windows + coverage + formatter + CodeFactor
    green. Codacy `action_required` (advisory, historically not blocking
    on this repo — same pattern as v1.3–v1.7). Combined status success.
- **Diff shape:** 18 files, +876/-612. Real code (excluding `.ts` churn
  and `DECISIONS.md`): ~165 LOC including the new 76-line unit test.
  Well under the 400-LOC cycle cap.
- **Translation cost:** **Zero** new hand-translated strings. The button
  tooltip reuses the existing "New Game" `tr()`, which is already
  translated 9/9 locales. 58/58 finished preserved per locale. First
  cycle in this project's history to ship user-visible UI with no
  translation delta.
- **Assumptions made:**
  - Emoji glyphs (🙂😎😵) render via Qt's font fallback on all three
    platforms (Apple Color Emoji / Segoe UI Emoji / Noto Color Emoji) —
    confirmed on macOS live; CI headless tests verify logic, not glyph.
  - Click handler reuses `MainWindow::onNewGame` — same semantics as
    `Game → New`, so no new telemetry event or stats path.
  - State→emoji mapping extracted to `smiley.h` as a header-only pure
    function so the unit test links against it without pulling
    `mainwindow.cpp` + `ui_mainwindow.h` + telemetry + language into
    the test target. Preserves the test-target minimalism precedent.
  - HBox stretch changed from `"1,1"` to `"1,0,1"` so the Fixed 32×32
    button sits centred between the flex-sized labels; `Qt::NoFocus`
    so Tab order skips it.
- **Skipped:**
  - *Distinct pressed-smiley (🫣) during click-and-hold on a cell.*
    Would need a new signal from `MineButton::cellPressed` → MainWindow
    and revert on release; adds a state the state machine doesn't own.
    Out of scope.
  - *Resetting on smiley click even mid-game without confirmation.*
    Matches classic behaviour (and `Game → New` does the same). No
    confirmation dialog added — consistent with existing UX.
  - *About/README screenshots update.* Cosmetic; would churn static
    assets for a feature users see immediately on launch.
- **Risks logged:** none new. Emoji font fallback is the only
  platform-dependent surface, and it uses the same glyphs the `🙂/😎/😵`
  literal already does in Qt's default text rendering.
- **Post-release watch (T+~5min):** Sentry `karaman/qminesweeper` —
  `search_issues` for unresolved issues in release `qminesweeper@1.8.0`
  in the last hour returned **zero results**. Expected — the assets
  were just published, telemetry is opt-in, and no install has had a
  realistic chance to fire a session yet. The signal worth watching for
  is *any* new group tagged with the 1.8.0 release; none observed.
  Watch closed.
- **Next candidates:**
  - Pause / resume (P shortcut) with board-covering overlay. Now the
    biggest remaining gap relative to classic clones.
  - Keyboard navigation (arrow keys + space/F) for accessibility.
  - Pressed-smiley (🫣) during cell click-and-hold — small follow-on
    polish on this cycle's feature.

## 2026-04-23 — Cycle 5 — v1.7.0 (autonomous)

- **Chosen problem:** No Custom difficulty. App shipped only
  Beginner / Intermediate / Expert; every mainstream Minesweeper clone
  ships Custom. Prior three cycles parked it as "multi-cycle because of
  Stats-schema ripples" — but v1.6.0 set a precedent by excluding Replay
  from Stats, which dissolves the multi-cycle concern for the same
  reason: Custom runs are a playground, not a record-setting mode.
- **Evidence:** `Difficulty` struct was already runtime-configurable;
  `MineField::newGame(Difficulty)` accepted any values;
  `refitWindowToContents()` already handled arbitrary grid sizes. The
  engine was ready; only UI + persistence + a stats-exclusion gate were
  missing.
- **Shipped:**
  - Branch: `feat/custom-difficulty` (merged + deleted)
  - PR: https://github.com/Mavrikant/QMineSweeper/pull/24 (squash-merged as `3cf6ea8`)
  - Release: https://github.com/Mavrikant/QMineSweeper/releases/tag/v1.7.0
  - CI: ubuntu + macos + windows + coverage + formatter + CodeFactor
    green. Codacy `fail` (advisory, historically not blocking on this
    repo — same pattern as v1.3/v1.4/v1.5/v1.6). Combined status success.
- **Diff shape:** 15 files, +1019/-516. Real code (excluding `.ts` churn
  and `DECISIONS.md`): ~200 LOC. Under the 400-LOC cap.
- **Translation cost:** 5 new hand-translated strings × 9 non-English
  locales (45 entries). 58/58 finished per locale, 0 unfinished —
  50/50 coverage preserved at the new string count.
- **Assumptions made:**
  - Custom games excluded from Stats (mirrors Replay precedent; full
    rationale in `DECISIONS.md`).
  - Width 9–30 / height 9–24 / mines ≤ w*h−9 — preserves 3×3 first-click
    safety zone.
  - Out-of-range `custom_*` plist values are silently clamped rather than
    surfacing a modal.
  - Custom sits inside the existing exclusive `m_difficultyGroup` rather
    than as a parallel non-checkable action — reuses the radio semantics
    users already know, with `recheckCurrentDifficultyAction()` handling
    the dialog-cancel revert.
- **Skipped:**
  - *Pause/resume.* Higher regression risk; parked for a 5th cycle.
  - *Keyboard navigation.* Zero translation cost but less user-visible
    than Custom for the same budget; parked for a 4th cycle.
  - *About/README updates mentioning Custom.* Would churn 10 existing
    long-string translations for a cosmetic line; not worth it.
- **Risks logged:** none new.
- **Post-release watch (T+~5min):** Release workflow run `24838989606`
  green in ~2 min; all 5 assets (Linux AppImage + tar.gz, macOS DMG,
  Windows ZIP, SHA256SUMS.txt) uploaded. Sentry shows 0 unresolved
  issues in release `qminesweeper@1.7.0` (expected — assets just
  published, zero downloads within the window this cycle could
  realistically cover). GitHub release body rewritten as user-facing
  prose covering the Custom dialog ranges, first-click-safety guarantee,
  stats exclusion, and platform-specific install notes (macOS quarantine
  clear).
- **Next candidates:**
  - Pause / resume (P shortcut) with board-covering overlay. Now the
    biggest remaining gap relative to classic clones.
  - Keyboard navigation (arrow keys + space/F) for accessibility.
  - Smiley-face status indicator (🙂/😎/😵) above the grid, clickable
    for new game — iconic Minesweeper UX that ours lacks.

## 2026-04-23 — Cycle 4 — v1.6.0 (autonomous)

- **Chosen problem:** No way to replay the mine layout you just played.
  Once a game ended, the board was gone — no second try on a fluke loss,
  no practice loop on a specific pattern, no "how fast can I solve *this*
  board" challenge. Classic speed-minesweeper clones (Minesweeper Arbiter,
  Minesweeper X, windows-minesweeper clones) all ship this; ours didn't.
- **Evidence:** `MineField::newGame(Difficulty)` always reseeded mines and
  there was no API to re-apply the previous mine set. No menu entry, no
  shortcut, no signal for "layout available".
- **Shipped:**
  - Branch: `feat/replay-same-layout` (merged via squash, then deleted)
  - PR: https://github.com/Mavrikant/QMineSweeper/pull/23 (merged as
    `808a51e`)
  - Tag: `v1.6.0`
  - Release: https://github.com/Mavrikant/QMineSweeper/releases/tag/v1.6.0
  - Release workflow: [run 24837942430](https://github.com/Mavrikant/QMineSweeper/actions/runs/24837942430)
    — all three platforms green in ~2m14s (13:27:34Z → 13:29:48Z UTC).
    Five assets published: `QMineSweeper-linux-x86_64.AppImage` (34.2 MiB),
    `QMineSweeper-linux-x86_64.tar.gz` (33.8 MiB),
    `QMineSweeper-macos-universal.dmg` (22.0 MiB),
    `QMineSweeper-windows-x64.zip` (42.0 MiB), plus `SHA256SUMS.txt`.
- **Diff shape (pre-merge):** 5 source files (`minefield.{h,cpp}`,
  `mainwindow.{h,cpp}`, `tests/tst_minefield.cpp`) + `CMakeLists.txt`
  version bump + 1 translation key × 9 locales + `DECISIONS.md` +
  `CYCLES.md`. Real code change ~130 LOC including tests. Under the
  400-LOC cycle cap.
- **Translation cost:** 1 new hand-translated string × 9 non-English
  locales. 50/50 coverage preserved.
- **Assumptions made:**
  - Replays do not update per-difficulty stats (Played / Won / Best).
    Reason: the board is no longer random — letting a memorised-board win
    set a new best time would devalue the leaderboard.
  - First-click safety is NOT re-applied on replay. If the user clicks a
    mine first, they lose immediately. That's the intended UX — they know
    the board.
  - Replay action enables on `gameStarted` (the exact moment `fillMines()`
    has populated the snapshot) and stays enabled through replays. Only
    `New` and `Difficulty` wipe it.
  - Shortcut is Ctrl+R (`QKeySequence::Refresh`) — cross-platform standard
    for "reload".
- **Skipped:**
  - *Per-layout best-time leaderboard.* Would need a hash of the mine
    positions + a new persistence schema. Out of scope.
  - *"Replay" visual indicator (e.g. a different window title).* The menu
    action remaining enabled is signal enough; no translation churn.
  - *Pause / resume, keyboard navigation.* Parked again — both higher
    regression risk than this cycle's budget.
- **Risks logged:** none new.
- **Post-release watch (T+~5min):** Sentry `karaman/qminesweeper` —
  `search_issues` for unresolved issues in release `qminesweeper@1.6.0`
  in the last hour returned **zero results**. No new crashes, no spike
  on prior groups attributable to the 1.6.0 cut. Telemetry is opt-in and
  the release just shipped, so the expected baseline volume is low; the
  signal worth watching for is *any* new group tagged with the 1.6.0
  release. None observed. Watch closed.
- **Next candidates:**
  - Pause / resume (P shortcut) with board-covering overlay.
  - Keyboard navigation (arrow keys + space/F) for accessibility.
  - "Games played today" mini-summary on the end-of-game dialog as a
    streak / session-activity hint (no new QSettings schema: derive from
    an in-memory counter since app launch).

## 2026-04-23 — Cycle 3 — v1.5.0 (autonomous)

- **Chosen problem:** No way to disable the `?` step in the right-click
  cycle. Classic Minesweeper ships a setting to opt out of it; many
  players find it an annoyance during fast play. Both the previous two
  cycles parked "Pause/Resume / Custom difficulty / Keyboard navigation"
  as next candidates — those are all bigger, higher-regression-risk
  changes; this cycle picks a small, self-contained usability feature
  the prior candidate list missed.
- **Evidence:** `MineButton::cycleMarker` unconditionally cycles
  `None → Flag → Question → None` with no escape hatch. Standard in
  Windows Minesweeper / GNOME Mines as an optional toggle; ours has no
  such toggle.
- **Shipped:**
  - Branch: `feat/question-marks-toggle` (merged + deleted)
  - PR: https://github.com/Mavrikant/QMineSweeper/pull/22 (squash-merged as `c7582ea`)
  - Release: https://github.com/Mavrikant/QMineSweeper/releases/tag/v1.5.0
  - CI: ubuntu + macos + windows + coverage + formatter green. Codacy
    flagged `action_required` (advisory, historically not blocking on
    this repo). Combined status success.
  - Release workflow: 2m27s (all three platforms parallel-built,
    macOS ad-hoc signed, SHA256SUMS attached).
- **Diff shape:** 20 files, +713/-482. Real code (excluding `.ts` churn
  and `DECISIONS.md`): ~180 LOC. Well under the 400-LOC cycle cap.
- **Translation cost:** 1 new hand-translated string × 9 non-English
  locales. 50/50 coverage preserved.
- **Assumptions made:**
  - Default `true` to preserve v1.4.x muscle memory — confirmed
    by reading `apply_translations.py` and by existing QSettings plist
    schema (absent key → read as `true`).
  - The setting is app-wide, not per-difficulty — matches Windows
    Minesweeper and GNOME Mines.
  - The About body ("right-click to flag") does not need a copy
    update — zero extra translation churn.
  - A static on `MineButton` is acceptable given the architectural
    constraint that MineButton has no back-pointer to MineField;
    documented in `DECISIONS.md`.
- **Skipped:**
  - *Pause/resume.* Higher regression risk on the timer/state machine;
    also adds ~3 new strings × 10 locales. Park.
  - *Custom difficulty.* Ripples into the Stats schema. Multi-cycle.
  - *Keyboard navigation.* Touches focus on every cell; reasonable
    candidate for a future cycle, but this cycle's feature is more
    user-visible for the same budget.
  - *About-dialog update about the new setting.* Kept the About body
    byte-identical to avoid touching 10 existing translations.
- **Risks logged:** none new.
- **Post-release watch (T+~5min):** Sentry shows 0 unresolved issues
  in release `qminesweeper@1.5.0`. Expected — the Linux/macOS/Windows
  assets were just published and have zero downloads. No regression
  signal within the window this cycle could realistically cover.
  Release workflow run `24835947800` green; all 5 assets + SHA256SUMS
  attached. GitHub release notes rewritten in user-facing prose.
- **Next candidates:**
  - Pause / resume (P shortcut) with board-covering overlay.
  - Keyboard navigation (arrow keys + space/F) for accessibility.
  - "Best time + date" moved from inline text to its own column in
    Statistics when Expert best time crosses 100s (readability tweak).

## 2026-04-23 — Cycle 2 — v1.4.0 (user-directed)

- **Trigger:** User-reported bug + follow-up feature ask: "when user
  changes game difficulty, a new game should start. currently game
  window size doesn't change" + "after fixing windows size issue,
  also add 'about' menu, show version and build information there.
  increment version, test, commit, push and release".
- **Chosen problem:** Bundle the window-refit bugfix with an About
  dialog enhancement (Qt version + build timestamp) into a single
  minor release.
- **Evidence (bug):** Reproduced live via computer-use. Starting on
  Beginner (9×9, 10 mines, ~120×140 window), switched to Expert — window
  stayed cramped while mine count flipped to 99. Symmetrically,
  Expert→Beginner left the window oversized.
- **Root cause (bug):** `MineField::setFixedSize()` posts
  `QEvent::LayoutRequest` to the *central widget*, not the MainWindow.
  The enclosing slot then read `sizeHint()` while the layout event was
  still in the queue, snapshotting the pre-change value. First attempt
  used `QCoreApplication::sendPostedEvents(this, LayoutRequest)` —
  targeted the wrong receiver, fixed only Beginner→Expert. Second
  attempt calls `centralWidget()->layout()->activate()` (synchronous
  re-layout of the right widget), then `adjustSize() + setFixedSize(sizeHint())`.
  Verified both directions in live app.
- **Shipped:**
  - Branch: `feat/resize-fix-and-about-build-info` (merged + deleted)
  - PR: https://github.com/Mavrikant/QMineSweeper/pull/21 (squash-merged as `0d4d018`)
  - Release: https://github.com/Mavrikant/QMineSweeper/releases/tag/v1.4.0
  - CI: all three platform builds + coverage + formatter green.
    Codacy `ACTION_REQUIRED` advisory — same state as v1.3.0, historically
    not blocking on this repo.
- **Translation cost:** 1 new hand-translated string × 9 non-English
  locales. The pre-existing about body was kept byte-identical by
  splitting it into a separate `tr()` so none of the existing
  translations went unfinished.
- **Assumptions made:**
  - `__DATE__` / `__TIME__` precision is sufficient build ID for the
    About footer — reproducibility-sensitive users can compare commit
    hashes.
  - Codacy ACTION_REQUIRED is advisory per prior cycle observation.
- **Skipped:**
  - *Compiler identity / build type in About.* Noise for most users;
    cost in new translatable strings > benefit.
- **Risks logged:**
  - macOS `.app` bundle startup: the fullscreen-off resize path now
    calls `centralWidget()->layout()->activate()` at startup too —
    unchanged behaviour since it's what was effectively happening
    implicitly; no regression in tests or live app.
- **Post-release watch (T+10min):** [to be filled]
- **Next candidates:**
  - Pause / resume (P shortcut) with board-covering overlay.
  - Custom difficulty dialog (width × height × mine count).
  - Keyboard navigation (arrow keys + space/F) for accessibility.

## 2026-04-23 — Cycle 1 — v1.3.0

- **Chosen problem:** Lifetime best-time records had no date context —
  you could see you'd done Beginner in 15.5 s, but not whether that was
  yesterday or three years ago. Small but real UX gap in the Statistics
  dialog added in v1.2.0.
- **Evidence:** Stats dialog (`showStatsDialog` in `mainwindow.cpp`)
  renders only `played / won / best time` with no timestamp; QSettings
  tree stored only `{played, won, best_seconds}`.
- **Shipped:**
  - Branch: `feat/best-time-date` (merged + deleted)
  - PR: https://github.com/Mavrikant/QMineSweeper/pull/20 (squash-merged as `170cf87`)
  - Release: https://github.com/Mavrikant/QMineSweeper/releases/tag/v1.3.0
  - CI: all three platform builds + coverage + formatter green; Codacy flagged
    `action_required` (advisory, historically not blocking on this repo) with
    no summary output. Not a blocker per combined status = success.
- **Post-release watch (T+10min):** Sentry shows 0 unresolved issues in
  release `qminesweeper@1.3.0`. Expected — the Linux/macOS/Windows assets
  were just published and have zero downloads. No regression signal within
  the window that this cycle could realistically cover.
- **Assumptions made:**
  - Day precision is sufficient (no wall-clock time) — the date is a
    memory anchor, not an audit trail.
  - Inline date rendering after the best-time cell avoids a new column
    header and therefore zero new translatable strings (important given
    the project's hand-translation-only policy for all 10 locales).
- **Skipped:**
  - *Per-locale date format override* — QLocale::ShortFormat respects
    the active QLocale already; no explicit override needed.
  - *"Best date" on the end-of-game "New record!" dialog* — cosmetic,
    not in this cycle's scope.
- **Risks logged:** none.
- **Next candidates:**
  - Pause / resume (P shortcut) with board-covering overlay. Real user
    pain for timed runs that get interrupted.
  - Custom difficulty dialog (width × height × mine count).
  - Keyboard navigation (arrow keys + space/F) for accessibility.
