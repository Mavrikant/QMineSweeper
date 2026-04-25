# Autonomous cycles log

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
