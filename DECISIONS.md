# Cycle decisions

## 2026-04-24 — Smiley status indicator (v1.8.0)

**Chosen:** Add a clickable smiley button above the minefield, between the
remaining-mines counter and the elapsed-time counter. It displays game state
via emoji — 🙂 while ready / playing, 😎 on win, 😵 on loss — and clicking
it starts a new game (equivalent to `Game → New`).

**Why this one:**
- Concrete user value — the yellow smiley-face reset button is one of the most
  recognisable visual elements of classic Minesweeper. Windows Minesweeper,
  Minesweeper Arbiter, GNOME Mines (kmines), and every clone on
  minesweepergame.com ship a clickable face as the primary "new game" gesture
  and the at-a-glance win/loss indicator. Ours had neither — the new-game
  affordance was hidden behind `Game → New` / `Ctrl+N`.
- Small, self-contained: ~30 LOC of real code in `mainwindow.{ui,cpp}` plus a
  one-function `smiley.h` header (pure state→emoji mapping) and a 3-test
  `tst_smiley.cpp`. No `minefield.cpp`/`minebutton.cpp` changes, no
  `QSettings` schema churn, no new dependencies.
- Low regression risk — the button is a consumer of existing
  `gameStarted`/`gameWon`/`gameLost` signals; the state machine itself is
  untouched. Clicking the button reuses `MainWindow::onNewGame()` — the same
  slot `Game → New` already calls. No new code paths through the state machine.
- Backwards compatible — pure UI addition; existing keyboard shortcuts and
  menus behave unchanged. The header layout still has the mine counter on the
  left and the timer on the right; the smiley is inserted centred between
  them with zero stretch so the two existing counters keep the same width
  allocation.
- Testable in isolation — factored the state→emoji mapping into a pure
  `smileyForState(GameState)` helper in `smiley.h`, which the new unit test
  exercises for all four `GameState` values including the transition paths.
- Low translation burden — 1 new hand-translated string × 9 non-English
  locales for the "New game" tooltip.

**Rejected alternatives from the prior `Next candidates` list:**
- *Pause / resume.* Higher regression risk on the timer/state machine and
  adds ~3 new strings × 10 locales (overlay text, resume button). Parked
  again — a fifth cycle deferral, but the risk profile hasn't changed.
- *Keyboard navigation.* Touches focus management on every cell; medium
  surface, zero translation cost, strong accessibility win but less
  immediately user-visible than the smiley for the same implementation
  budget. Parked for a future cycle.

**Implementation choices:**

1. **Emoji text on a `QPushButton`, not image assets.** The three v1.8.0
   states reuse the same Unicode code points already proven in the
   end-of-game dialog (🏆 since v1.2.0). Qt's font-fallback stack handles
   these consistently across macOS (Apple Color Emoji), Windows (Segoe UI
   Emoji since Windows 10), and Linux (Noto Color Emoji on every modern
   distro). Shipping PNG assets for four states × three resolutions would
   balloon the resource bundle for no visual win.

2. **State→emoji mapping lives in `smiley.h` as an inline pure function,
   not a static member of `MainWindow`.** Avoids pulling `mainwindow.cpp`
   (and the `.ui`-generated header + `telemetry.cpp` + `language.cpp` +
   `QMS_VERSION` define + `resources.qrc`) into the test target just to
   exercise a four-case switch. The test links the existing
   `QMineSweeperCore` plus the one-header helper.

3. **Click handler reuses `MainWindow::onNewGame()` directly.** No separate
   code path — the button click is semantically identical to invoking
   `Game → New`. Both breadcrumbs and QSettings updates flow through the
   same slot; there is no way for the two entry points to drift.

4. **Smiley is centred with zero stretch inside the header row.** Changed
   the `horizontalLayout` stretch from `1,1` (mine counter and timer each
   claim half) to `1,0,1` (mine counter and timer keep their share of the
   width; the smiley takes exactly its sizeHint in the middle). This keeps
   the header compact on Beginner's narrow window and the timer/counter
   still right-aligned on wider difficulties.

5. **Button is square and fixed-size (32×32 px) so it doesn't visually
   fight with the counter labels.** `sizePolicy=Fixed` prevents layout
   engines on different platforms from stretching it asymmetrically.

6. **No stats or telemetry tagging on the button click.** Clicking the
   smiley is indistinguishable from `Game → New`; we already breadcrumb
   `ui: new game` in `onNewGame`. No separate `ui: smiley clicked` event —
   counting two sources of the same action would inflate the metric.

**Assumptions:**
- Emojis render consistently. Validated by the fact that 🏆 has shipped
  without rendering complaints in Sentry or GitHub issues since v1.2.0.
- A tooltip ("New game") is enough accessibility; no additional aria-role
  or keyboard shortcut on the button itself — `Ctrl+N` already works via
  the menu and the visual button is redundant for keyboard users.
- The middle-click "tension face" (😮 while a cell is pressed) common to
  Windows Minesweeper is excluded from v1.8.0. It requires hooking into
  `MineButton::cellPressed` + mouse-release propagation, which is
  materially bigger than the win/loss indicator alone. Park for later if
  a user actually asks.
- Changing the horizontalLayout stretch from `1,1` to `1,0,1` does not
  break existing behaviour since neither counter was visibly stretching
  before (both are fixed-size QLabels).

## 2026-04-23 — Custom difficulty (v1.7.0)

**Chosen:** Add `Game → Difficulty → Custom…` — a small dialog that lets the
player pick an arbitrary grid (width 9–30, height 9–24, mine count 10 up to
`w*h − 9`). The geometry persists across launches under
`settings/custom_{width,height,mines}`; startup restores a prior Custom the
same way it restores Beginner/Intermediate/Expert.

**Why this one:**
- Concrete user value — Custom is a staple of every mainstream Minesweeper
  clone (Windows Minesweeper, GNOME Mines, Minesweeper X). Ours had only the
  three presets, and the `Difficulty` struct already accepts arbitrary
  values — the feature was one dialog and one radio entry away.
- Small, self-contained: ~140 LOC of real code in `mainwindow.{h,cpp}` plus
  3 new unit tests. No `minefield.cpp`/`minebutton.cpp` changes — the
  underlying engine is already size-agnostic.
- Backwards compatible — default behaviour is unchanged. Existing
  `difficulty=Beginner|Intermediate|Expert` plists restore exactly as before.
  Out-of-range values in a hand-edited plist are clamped to the same bounds
  the dialog enforces, so QSettings can't crash the startup path.
- Testable in isolation — 3 new unit tests (arbitrary-sized grid, first-click
  safety on 15×12, and the dense-packing boundary 72-mine-on-9×9 that
  exercises the relaxed-exclusion branch in `fillMines`).
- Translation burden — 5 new hand-translated strings × 9 non-English locales.

**Rejected alternatives from the prior `Next candidates` list:**
- *Pause / resume.* Still bigger surface (overlay widget, timer arithmetic,
  ~3 new strings × 10 locales) and higher regression risk on the most
  critical UI path. Park again.
- *Keyboard navigation.* Touches focus management on every cell. Medium
  surface, zero translation cost. Reasonable candidate for a future cycle
  but less user-visible than Custom for the same implementation budget —
  parked for a fourth cycle in a row.
- *Best-time-plus-date column in Statistics when Expert time crosses 100s.*
  Purely cosmetic. Parked.

**Implementation choices:**

1. **Stats policy — Custom games do NOT update `Played`, `Won`, or
   `Best time`.** This mirrors the precedent v1.6.0 set for Replay: the
   per-difficulty best-time leaderboard is only meaningful across the three
   standard presets. A win on 12×12/30-mines is not comparable to a win on
   30×24/400-mines; lumping them into a single `"Custom"` row would produce
   nonsense leaderboards, and keying Stats by `"Custom WxH×M"` would
   multiply the Statistics dialog's row count without bound. Telemetry
   events still carry `width`/`height`/`mines` so custom-config use is
   observable in Sentry without touching Stats.

2. **Max mine count = `w*h − 9`.** Preserves the 3×3 first-click safety-zone
   guarantee by leaving at least 9 safe cells. On 9×9 that caps at 72 — a
   valid "evil" configuration the existing `relaxExclusion` branch in
   `fillMines` handles. The dense-packing unit test exercises exactly this.

3. **Custom sits inside the existing exclusive `m_difficultyGroup`, after a
   menu separator.** On trigger it opens the dialog; on cancel,
   `recheckCurrentDifficultyAction` flips the tick back to whatever
   difficulty is actually active (since `QActionGroup` already moved the tick
   to Custom when the user clicked). Cleaner than a parallel non-checkable
   action because it reuses the radio semantics users already know.

4. **Startup restore with clamping.** `std::clamp` applied to every
   `custom_*` settings read — guards against hand-edited plists with
   out-of-range values. A missing key falls through to Expert-sized defaults
   (30/16/99).

5. **Window refit is unchanged.** `refitWindowToContents()` already handles
   arbitrary grid sizes since v1.4.0; no new code path.

**Assumptions:**
- Custom excluded from Stats (decision 1 above). Ruled against "count them
  under a single Custom row" because the stats would be uninterpretable, and
  against "key Stats by dimensions" because it dilutes the leaderboard and
  multiplies the row count. The three standards remain the records people
  care about.
- Width 9–30 matches Expert's width; height 9–24 gives more vertical room
  than Expert (16) without exceeding common screen heights at `CellSize=30`
  (24 × 30 = 720 px — still fits 1080p comfortably with menu/margins).
- Clamping over error dialogs for malformed plists — users who edit the plist
  by hand should not see a modal; defaulting quietly is correct UX.

## 2026-04-23 — Replay same layout (v1.6.0)

**Chosen:** Add `Game → Replay same layout` (Ctrl+R / `QKeySequence::Refresh`) so
players can re-attempt the exact board they just played — to practise a specific
mine pattern, improve their time on a hard fluke, or just keep going on a lucky
layout.

**Why this one:**
- Concrete user value — replaying a layout is a staple of speed-minesweeper
  (Minesweeper Arbiter, Minesweeper X and the windows-minesweeper clones all
  ship it). Our app had no equivalent; once a game ended, the layout was gone.
- Small, self-contained: ~60 LOC of real code. One new `MineField` method
  (`newGameReplay`), one new getter (`canReplay`), one new vector member
  (`m_lastMinePositions`), one new `QAction` in MainWindow, and an `m_isReplay`
  flag to gate stats.
- Backwards compatible — default behaviour is unchanged; `newGame(Difficulty)`
  still wipes any prior layout. No new QSettings key. Existing saves untouched.
- Testable in isolation — added 6 unit tests covering `canReplay()` state,
  mine-position preservation, cell reset, post-loss replay, and the
  "no layout yet" fallback.
- Low translation burden — 1 new hand-translated string × 9 non-English locales.

**Rejected alternatives from the prior `Next candidates` list:**
- *Pause / resume.* Bigger surface (overlay widget, timer arithmetic, ~3 new
  strings × 10 locales), highest regression risk on the most critical UI path
  (timer / state machine). Parked again for a cycle that is willing to
  absorb that risk.
- *Keyboard navigation.* Touches focus management on every cell; medium
  surface, useful but less user-visible than replay for the same budget.
- *Best-time-plus-date column in Statistics when Expert time crosses 100s.*
  Purely cosmetic; parked.

**Implementation choices:**

1. **Stats policy — replays do NOT update `Played`, `Won`, or `Best time`.**
   Rationale: the user has already seen the mine positions; counting a replay
   win would let you inflate your best time by memorising an easy board.
   Telemetry still records `game.won`/`game.lost` with a `replay=true` tag
   so we can see how often replay is actually used in the wild without
   contaminating the per-difficulty leaderboard.

2. **First-click safety is NOT re-applied on replay.** If the user clicks a
   mine on the first click of a replay, they lose immediately. That's the
   whole point — they know where the mines are. `m_minesPlaced` is set to
   `true` on replay so `fillMines()` is skipped.

3. **Menu action enable/disable.** Disabled initially (no layout yet).
   Enabled on the `gameStarted` signal — the exact moment `fillMines()` has
   populated `m_lastMinePositions`. Cleared back to disabled by `onNewGame()`
   and `onDifficultyChanged()`. Replaying keeps it enabled (the layout is
   still available).

4. **Fallback when `canReplay()` is false.** `newGameReplay()` returns `false`
   and delegates to `newGame(m_difficulty)`. The menu action is disabled in
   that state anyway, but the fallback makes the method safe to call
   unconditionally — useful for tests and for a possible future "R hotkey
   always works" tweak.

5. **Shortcut is Ctrl+R (`QKeySequence::Refresh`).** Standard cross-platform
   "reload/restart" gesture. F5 also triggers `Refresh` on most platforms;
   users on either muscle memory get it for free.

**Assumptions:**
- Replays should not count in stats (decision 1 above). Ruled against the
  alternative ("count them — the user still played") because the recorded
  best-time per difficulty is the project's only leaderboard-style metric.
- Not gating the action on game-over: you can replay even while still playing.
  That's consistent with "New game" also being available mid-game.
- `m_lastMinePositions` is cleared on difficulty change even if the user
  never clicked a cell — no layout was ever generated. Keeps invariants simple.

## 2026-04-23 — Question-marks toggle (v1.5.0)

**Chosen:** Add `Settings → Enable &question marks` so users can opt out of the
None → Flag → **Question** → None right-click cycle and play with the classic
two-state None → Flag → None instead.

**Why this one:**
- Concrete user value — question marks are a common Minesweeper annoyance for
  fast play; both Windows Minesweeper and GNOME Mines ship this setting. The
  feature is a standard expectation for a minesweeper app.
- Small, self-contained: ~50 LOC of real code, one new QAction, one QSettings
  key, and a single MineField sweep helper.
- Backwards compatible — default `true` preserves v1.4.x behaviour byte-for-byte.
  An existing plist with no `settings/question_marks` key reads as `true`.
- Testable in isolation — MineButton's right-click cycle is already unit-tested;
  I add two cases for the disabled branch and the sweep helper.
- Low translation burden — 1 new key × 9 non-English locales, hand-written.

**Rejected alternatives from the prior `Next candidates` list:**
- *Pause / resume.* Larger surface (overlay widget, timer arithmetic, ~3 new
  strings × 10 locales), touches the most critical UI path (the timer / state
  machine). Higher regression risk than a cycle with no human review can absorb.
  Park again.
- *Custom difficulty dialog.* Ripples into the Stats schema (best-time keyed
  by preset name today; custom configs don't fit). Multi-cycle. Park.
- *Keyboard navigation.* Touches focus on every cell. Medium surface, zero
  translation cost — reasonable candidate for a future cycle, but this cycle's
  feature is more user-visible for the same implementation budget.

**Implementation note — why a static on MineButton:**
The `CLAUDE.md` architecture explicitly states MineButton has *no back-pointer
to MineField* ("signals up, slots down"). A per-instance setting plumbed via
MineField would violate that direction. The toggle is app-wide, not per-cell
or per-game, so a static member on MineButton (hidden behind a getter/setter)
is the natural fit. The setting is owned by MainWindow (persists to QSettings,
drives the checkable QAction); MineButton just reads the static.

**Mid-game sweep:**
When the user toggles OFF while a live board has `?`-marked cells, those cells
would otherwise stay stuck in Question until the user right-clicks them again.
`MineField::clearAllQuestionMarks()` sweeps the grid once to reset them to
None. No `flagToggled` emission — Question → None is already a silent
transition in the existing state machine.

**Assumptions:**
- Default `true` is correct for existing users (preserve v1.4.x muscle memory).
- The setting is app-wide, not per-difficulty (Windows Minesweeper does it the
  same way).
- No about-dialog update needed: the About body already describes right-click
  as "flag" without mentioning question marks, so no translation churn.

## 2026-04-23 — Window-refit + About build info (v1.4.0)

**Chosen:** Fix the window-refit-on-difficulty-change bug (user-reported) and,
in the same cycle, enhance Help → About with Qt version + build timestamp
(user follow-up).

**Bug root cause (resize):** `MineField::setFixedSize()` posts
`QEvent::LayoutRequest` *to the central widget* (not to MainWindow). The
enclosing slot then called `adjustSize()`/`setFixedSize(sizeHint())` while
the event was still queued, so `sizeHint()` returned the pre-change value.
Net effect: Beginner → Expert stayed small (old small hint), Expert →
Beginner stayed large (old large hint). First attempt flushed with
`sendPostedEvents(this, LayoutRequest)` — targeted the wrong widget,
fixed only one direction. Final fix calls `centralWidget()->layout()->activate()`
to re-run layout synchronously, then snapshots the updated `sizeHint()`.

**Rejected alternatives (resize):**
- *Relax to non-fixed resizable window.* Would sidestep the bug but loses the
  pixel-perfect "board fills window exactly, no empty gutter" UX.
- *QTimer::singleShot(0, ...) defer.* Would work but leaves a visible flicker
  and makes the slot async — synchronous layout activation is cleaner.

**About dialog choice:** Show Qt version (`QT_VERSION_STR`) + build timestamp
(`__DATE__ " " __TIME__`). Deferred compiler identity and build type — nice
to have but noise for most users and require extra macros.

**Why ship them together:**
- Both small, both touch `mainwindow.cpp`, both merit one release bump.
- Splitting into two PRs doubles review/CI cost for no user benefit.

**Translation preservation trick:** Split the About body into two `tr()`
calls so the large pre-existing about string literal stays byte-identical.
That keeps all 10 locales' previous translation intact; only the new
"Built with Qt %1 on %2" string needs fresh hand-translation. Net
translation surface: 1 new key × 9 non-English locales.

**Assumptions:**
- `__DATE__` / `__TIME__` precision (date + HH:MM:SS) is adequate build-ID
  for an end user; reproducibility-minded users can compare commit hashes.
- Small-font footer is styling-neutral enough that no per-locale tweaks
  are needed.

## 2026-04-23 — Best-time date in Statistics (v1.3.0)

**Chosen:** Record the date a best-time was set and show it inline in the
Statistics dialog after the best-time value.

**Rejected alternatives:**
- *Pause / resume* — larger surface (overlay widget, timer arithmetic, three new
  user-facing strings across 10 locales), higher risk of regression in the
  existing timer/state machine. Park for a later cycle.
- *Custom difficulty* — introduces a second axis (what do we do with stats for
  arbitrary grid configs?) and more UX surface. Park for a later cycle.
- *Keyboard navigation* — touches focus management on every cell; nice but not
  the biggest lever and the current mouse UX is fine.

**Why this one:**
- Concrete user value — a lifetime record is much more meaningful when you can
  see *when* you set it; serves as a "last played" memento for casual players.
- Backwards compatible — a Record written by v1.2.0 has no `best_date`; we
  render a blank date for that case. No migration needed.
- Testable via the existing `Stats` unit suite — time/date is injectable.
- Ships with zero new translatable strings — the locale-formatted date goes
  inline in the existing *Best time* cell (e.g. `15.5 s (2026-04-23)`),
  avoiding the "new strings ship English-only to 9 locales" footgun.

**Assumptions:**
- Date-only precision is sufficient (no hour/minute). Reasoning: the "when" is
  for memory anchoring, not audit — day granularity is enough and keeps the
  cell short.
- ISO 8601 (`yyyy-MM-dd`) is the canonical persistence format; display uses
  `QLocale::toString(date, QLocale::ShortFormat)` so the user sees a familiar
  format in their locale.
- `resetAll()` already wipes the `stats/` settings group, so the new
  `best_date` key is cleared automatically — no code change needed for reset.
