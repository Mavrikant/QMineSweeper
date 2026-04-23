# Cycle decisions

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
