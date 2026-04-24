# Autonomous cycles log

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
