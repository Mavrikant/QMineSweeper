# Cycle decisions

## 2026-04-25 — Pause / resume (v1.12.0)

**Chosen:** Add a pause/resume toggle that freezes the game timer, blocks
all minefield input (mouse + keyboard), and dims the board with a
"Paused" overlay until the user resumes. Action lives at `Game → Pause`
(toggling label "Pause" ↔ "Resume") with the `P` shortcut. Only
available while a game is in progress (`GameState::Playing`).

**Why this one:**
- Parked for **seven cycles running** (3, 4, 5, 6, 7, 8, 9). Every
  prior cycle's "Next candidates" list led with this exact item:
  *"Pause / resume (P shortcut) with board-covering overlay."* The only
  recurring rejection reason was timer/state-machine risk in autonomous
  mode — the rest of the queue (no-flag achievement, tutorial-overlay
  upgrade, custom difficulty) has been smaller-value or already shipped.
  At cycle 10, the parked item is the highest-value remaining feature
  and the timer-offset surgery is contained enough to absorb safely.
- Real user pain — timed runs that get interrupted (phone, doorbell,
  Slack ping) currently force the user to abandon a winning Expert run
  or watch their best-time count climb past truth. Standard in GNOME
  Mines; Windows Minesweeper Classic shipped pause via the menu too.
- Self-contained surface: no schema changes, no QSettings keys (pause
  state is in-memory only — restarting the app starts a fresh game),
  no telemetry events of consequence (a `pause` breadcrumb is enough).

**Rejected alternatives from the prior `Next candidates` list:**
- *No-flag speedrun achievement.* Genuinely small (1 string × 9 locales,
  ~50 LOC). Would have been the contained autonomous default — but
  pause/resume strictly dominates on user-visible value. Park.
- *Overlay-with-bubbles tutorial upgrade.* Cosmetic; nobody has
  complained since v1.10.0 shipped the dialog-style tutorial. Park.
- *Custom difficulty.* Already shipped in v1.7.0 — the cycle-9 next
  list inherited a stale entry. Drop.

**Implementation choices:**

1. **No new GameState enum value.** Pause is a *cross-cutting* freeze
   on input + timer, not a separate machine state. `Playing` remains
   `Playing` while paused — the win/loss invariants are unchanged, and
   a parallel `m_paused` flag in `MineField` and `MainWindow` is the
   minimal-touch path. Adding `GameState::Paused` would have rippled
   into `MineField::checkWin`, `onChordRequested`, `setSmileyState`,
   and every test that asserts a state. Far costlier than a boolean.

2. **Block input via `MineField::eventFilter`, not `setCellEnabled`.**
   `setCellEnabled` is the win/loss freeze path — it disables the
   button-level `m_enabled` flag and changes the cursor to arrow,
   which is the right cue for game-over but wrong for pause (the cells
   are *not* permanently dead; they're just temporarily ignored). The
   eventFilter already exists for keyboard nav (cycle 9); adding a
   single `if (m_paused) swallow` guard in front of MouseButtonPress /
   MouseButtonRelease / MouseButtonDblClick / KeyPress / KeyRelease is
   ~6 lines and keeps cells visually unchanged so the user sees their
   board exactly as it was.

3. **Overlay is a child of `MineField` itself.** A `QFrame` parented
   to the minefield, geometry locked to the field's full rect, with
   a centered "Paused" label and a translucent dim background. Sits
   on top of all cells in the Z-order, so even without the eventFilter
   guard it absorbs mouse events naturally — the eventFilter is
   defence-in-depth for keyboard/focus paths the overlay can't catch.

4. **Timer offset, not pausable elapsed timer.** `QElapsedTimer` does
   not expose a pause API. Track `m_pausedTotalMs` (cumulative paused
   milliseconds across multiple pause/resume cycles in one game) and
   `m_pauseStartMs` (the `m_gameTimer.elapsed()` value at the moment
   pause began). On resume, add `(elapsed() - pauseStartMs)` to
   `pausedTotalMs`. `elapsedSeconds()` subtracts both the running
   pause segment (if active) and `pausedTotalMs`. Reset to zero on
   every `newGame` / `newGameReplay` / difficulty change.

5. **`P` shortcut at WindowShortcut context.** Single-key `P` (no Ctrl)
   matches Windows Minesweeper Classic and GNOME Mines convention.
   `WindowShortcut` (the QAction default) fires BEFORE focused-widget
   keyPress handlers, so a focused cell does not need to know about
   the shortcut and the eventFilter doesn't need a `Key_P` case.

6. **Pause auto-clears on game end / new game / replay / difficulty.**
   Any state transition that resets the timer also resets the pause
   accumulator. Eliminates the "I changed difficulty mid-pause and now
   the new game starts paused" failure mode.

7. **No persistence.** Pause is an in-session concept. Saving "I was
   paused at 12.4s" to QSettings would imply game-state persistence
   across launches — a much bigger feature ("save and resume games")
   that was not asked for. If the user quits while paused, the game
   is lost. Documented in the PR.

8. **No telemetry event for pause itself, just a breadcrumb.** Pause
   doesn't change the `game.won` / `game.lost` schema. The duration
   tagged on those events still uses `elapsedSeconds()` post-offset,
   so a paused-then-resumed Expert run records its true playing time,
   not wall-clock time. Adding a `game.paused` event would balloon the
   Sentry quota for a low-signal lifecycle hook. A breadcrumb gives
   us the same crash-context value at a fraction of the cost.

## 2026-04-24 — Keyboard navigation (v1.11.0)

**Chosen:** Add full keyboard control of the minefield — arrow keys move
focus between cells, Space / Enter reveal (or chord if the focused cell
is already opened), F toggles the marker (same cycle as right-click),
and D forces chord. The focused cell gets a distinct blue inset focus
ring so keyboard users can see where they are.

**Why this one:**
- Parked for five cycles running (3, 4, 5, 6, 7, 8) with the same
  rationale each time: "medium surface, zero translation cost, good
  accessibility win." Nothing else on the parked list has a lower
  translation burden, and the repo's user-feedback queue is empty — so
  autonomous cycle 9 is the right moment to clear it.
- Accessibility: today the app is unplayable without a mouse. A
  minesweeper grid is a pure keyboard-friendly UI (finite cells,
  discrete actions) — there's no good reason to gate it on mouse input.
- Small, self-contained: ~90 LOC of real code (MineButton focus policy
  + paint ring; MineField eventFilter with a switch-statement dispatcher;
  no new QSettings keys, no new menu items, no new dialog). Tests:
  ~130 LOC exercising all eight key paths. Under the 400-LOC cycle cap.
- **Zero new translatable strings.** The feature has no visible label,
  tooltip, menu entry, or dialog copy — it's keyboard behaviour plus a
  focus outline. 81/81 finished per locale preserved.

**Rejected alternatives from the prior `Next candidates` list:**
- *Pause / resume.* Still the highest-value parked candidate for a
  human-directed cycle, but ~3 new strings × 10 locales and it touches
  the timer/state machine — the most critical UI path. Sixth cycle of
  parking; still the right call for an autonomous budget.
- *No-flag speedrun achievement.* Genuinely small. Would add 1 new
  translatable string and a Stats-schema column (best no-flag time).
  Parked — keyboard nav strictly dominates on both accessibility value
  and translation cost (zero vs. one).
- *Overlay-with-bubbles tutorial upgrade.* Cosmetic; nobody has asked.

**Implementation choices:**

1. **Event filter on MineField, not keyPressEvent on MineButton.** The
   project's bedrock architecture rule is "MineButton has no back-pointer
   to MineField — signals up, slots down." Handling arrow keys requires
   knowing the grid to resolve "the cell one row up" — a lookup that
   only MineField can do. An event filter on the parent wrapping every
   child runs BEFORE the child's own keyPressEvent, which is the only
   way to intercept Space/Enter before `QAbstractButton::keyPressEvent`
   emits `clicked` (which nothing listens to, but that's beside the
   point — we want our reveal-vs-chord decision, not the default).

2. **Qt::StrongFocus explicit on MineButton.** macOS ships
   `Qt::TabFocus` by default for push buttons, which makes arrow-key
   navigation work on Linux/Windows but silently no-op on macOS. An
   explicit setter rules out platform drift.

3. **Focus ring drawn in `paintEvent` override, not `:focus` stylesheet
   pseudo-state.** The cell stylesheet uses `border: 0px` (to get the
   flush checkerboard look) and changes mid-game (opened / mine /
   wrong-flag). Chaining `:focus { border: ... }` onto every stylesheet
   mutation would be a correctness landmine. A single `paintEvent`
   override that draws a 2-px inset rectangle when `hasFocus()` is
   stylesheet-independent and survives every state change.

4. **Space dispatches reveal-vs-chord based on opened state.**
   - Opened cell → `onChordRequested(r, c)` (same as middle-click).
   - Unopened, non-flag cell → `cell->Open()` (same as left-click).
   - Unopened, flagged cell → no-op (flag protects from reveal, same
     as left-click).
   This mirrors the "intuitive single key" UX of GNOME Mines —
   keyboard users don't want to remember separate keys for reveal
   and chord when the cell state already disambiguates.

5. **`D` forces chord.** Kept for players who memorise the opened
   state and want an explicit chord key (parallels middle-click). On
   an unopened cell D is a no-op, not an error — minimises surprise.

6. **`F` reuses `cycleMarker()`, which is moved from `private` to
   `public` on MineButton.** `cycleMarker` is idempotent with the
   mouse right-click path — same state transitions, same
   `flagToggled` signal emissions, same question-marks setting
   respect. Making it public is a trivially safe visibility widen;
   the alternative (synthetic `QMouseEvent` injection) is ugly.

7. **Key events during `GameState::Won`/`Lost` only allow arrow
   navigation.** After a game ends, `freezeAllCells()` disables cell
   actions. The event filter short-circuits Space/F/D/Enter at the
   top of `handleCellKey` when the state is terminal; arrows still
   work so the user can look around the revealed board.

8. **No auto-focus on new game.** Set-focus-on-build would steal
   focus from the menu bar and the telemetry-consent dialog on
   startup. Users who want keyboard control Tab into the grid or
   click a cell first, then navigate. Matches how every other
   keyboard-friendly Qt widget behaves.

**Assumptions:**
- QAbstractButton does not pre-consume arrow keys outside a
  QButtonGroup (confirmed empirically — arrow keys reach the event
  filter even without it returning true early). The filter still
  handles them unconditionally, so this is defence-in-depth.
- The inset focus ring (2-px blue border at 1-px inset) is visible
  on every cell stylesheet: green-checker base, tan-checker opened,
  orange mine-reveal, red wrong-flag. Verified in live app on the
  green+tan combination; the blue is high-contrast against all four.
- `D` is not a common muscle-memory shortcut for any minesweeper
  clone (Windows Minesweeper uses middle-click only). Picked because
  it's adjacent to F on QWERTY — easy to remember as the "other
  modifier-free action key."

## 2026-04-24 — First-run tutorial (v1.10.0, closes #26)

**Chosen:** A six-step modal-card tutorial opens once automatically on the
first launch for each install, and is re-openable any time via
`Help → Tutorial`. Skip and Finish both mark the tutorial completed so
the next launch doesn't re-prompt; the Help menu is the escape hatch.

**Why this one:**
- **It's the one open GitHub issue.**
  [#26](https://github.com/Mavrikant/QMineSweeper/issues/26) — filed by
  the repo owner — asks for exactly this. Closing a product-owner
  request beats pulling another parked candidate from the cycle log.
- Small, self-contained: ~100 LOC of tutorial module + ~80 LOC of tests
  + ~30 LOC of MainWindow wiring. No `minefield.cpp`/`minebutton.cpp`
  changes, no state-machine surface, no new timers, no new telemetry
  schema. Translation surface is 19 strings × 9 locales.
- Backwards compatible — pure additive UI. Existing installs get the
  tutorial once (their plist has no `tutorial/completed` key) and then
  behave identically to v1.9.0.
- Testable in isolation — `tst_tutorial.cpp` exercises step-list shape,
  Back/Next navigation, Finish-on-last-step completion signal,
  Skip-emits-skipped signal, and QSettings persistence, all without
  touching `mainwindow.cpp` or the `.ui`-generated header.

**Rejected alternatives:**
- *Pointing-bubble overlay that highlights each UI element.* Polished
  but ~400 LOC of custom painting + target-widget geometry tracking.
  Doesn't earn the budget for a one-shot feature. A future cycle can
  upgrade if users or Sentry surface actual complaints.
- *Replaying the existing consent/language prompts as a "welcome wizard"
  sequence.* Would conflate two user decisions (privacy choice vs. "how
  do I play"). Kept them separate — consent first, tutorial after.
- *Shipping the pointing-bubble version as a Settings toggle.* Adds a
  settings key for a UI flavour 99% of users will never touch. No.

**Implementation choices:**

1. **`Tutorial::steps()` returns a static `QVector<Step>` of
   `{const char *title, const char *body}` wrapped in `QT_TR_NOOP`.**
   Same pattern as the difficulty menu (`mainwindow.cpp` lines ~121–135)
   and the existing Stats rows. `lupdate` extracts the literals; the
   runtime `tr(raw)` at the use site resolves them via the installed
   translator. A single place to edit step content.

2. **`TutorialDialog` is a plain `QDialog`, not a `QMessageBox`.** Body
   text is long enough to need word-wrap and a minimum width; the
   built-in buttons need to be three not two (Back / Next / Skip with
   Back disabled on step 1 and Next turning into "Finish" on the last
   step). A `QDialog` with a hand-laid-out button row is cleaner than
   fighting `QMessageBox`'s standard-button bitmask.

3. **Skip marks completed the same way Finish does.** If the user
   Skips once and then changes their mind, they can always re-open it
   from `Help → Tutorial`. Any more nuance (e.g. "remind me next
   launch") adds a third state machine position for a single-bit user
   decision.

4. **Deferred first show via `QTimer::singleShot(0, this,
   &MainWindow::showTutorialDialog)`.** Stacking two `exec()`-modal
   dialogs inside the ctor (consent prompt + tutorial) works, but the
   deferred show lets the main window paint first so the tutorial
   lifts over a visible board rather than an empty grey frame.

5. **`closeEvent` emits `skipped()` unless `completed()` already fired.**
   Matches classic "close-box == cancel" behaviour; the QSettings flag
   still flips so the close-box isn't a re-prompt loophole.

6. **Help-menu action inserted *before* About.** `Tutorial` at the top
   of `Help` is the expected hierarchy (most-useful first).

**Assumptions:**
- Six steps is the right length. Any fewer and the chord-click + "?"
  mechanics get skipped; any more and the text fatigue outweighs the
  payoff. Matches how Windows Minesweeper, Minesweeper Arbiter and
  GNOME Mines all do it.
- The chord step mentions "left+right together" because the middle
  button is unreliable on modern MacBooks. Sentry has no platform
  breakdown yet for who uses which chord input; this copy is the
  safest bet.
- `tutorial.completed` is intentionally app-wide (not per-difficulty
  or per-language) — the mechanics don't change per preset.

## 2026-04-24 — Tension smiley during cell hold (v1.9.0)

**Chosen:** While a cell is being held down during an active game, the header
smiley flips to 😮 — the classic "holding my breath" / "tension" face —
reverting to 🙂 / 😎 / 😵 on release. Left-click and middle-click (and L+R
chord) trigger tension; right-click flag cycling does not.

**Why this one:**
- Concrete user value — the tension face is the third leg of the
  classic-Minesweeper feedback loop, right alongside the static 🙂/😎/😵 we
  shipped in v1.8.0. Windows Minesweeper, Minesweeper Arbiter, Minesweeper X,
  GNOME Mines, and every clone on minesweepergame.com show it. Players expect
  it; the only thing the board was missing.
- Explicitly called out in v1.8.0's `Next candidates` list as "Pressed-smiley
  (🫣) during cell click-and-hold — small follow-on polish on this cycle's
  feature." Zero new territory — the state machine is untouched, the UI slot
  already exists, and emoji rendering is proven since v1.2.0.
- Small, self-contained: ~80 LOC of real code across MineButton / MineField /
  smiley / MainWindow plus ~125 LOC of tests (5 new MineButton cases, 3 new
  smiley cases including a full press→release integration through a real
  MineField). Total diff 209 insertions / 2 deletions, well under the 400-LOC
  cycle cap.
- Backwards compatible — pure UI addition. No settings, no telemetry, no
  QSettings schema change. The existing signal `cellPressed` and its
  first-click-placement role are untouched; the new signals are separate.
- Testable in isolation — `smileyForTensionState(GameState, bool pressing)`
  is a pure inline helper the unit test exercises without pulling any Qt
  widget code.
- Translation burden — **zero**. 😮 is a Unicode glyph on the same font
  fallback stack that has been rendering 🙂/😎/😵 since v1.8.0 and 🏆 since
  v1.2.0 without a single rendering complaint in Sentry or GitHub issues.

**Rejected alternatives from the prior `Next candidates` list:**
- *Pause / resume.* Still bigger surface (overlay widget, timer arithmetic,
  ~3 new strings × 10 locales) and higher regression risk on the timer/state
  machine. Park for a sixth cycle.
- *Keyboard navigation.* Touches focus management on every cell. Medium
  surface, zero translation cost. Reasonable next candidate but less
  immediately user-visible than the tension face for the same budget.

**Implementation choices:**

1. **Two new signals on `MineButton` — `pressStart()` / `pressEnd()` — cell
   agnostic on purpose.** The header indicator doesn't care *which* cell is
   being held, only *that* one is. Emitting `pressStart(row,col)` would force
   MainWindow to ignore the args and would give the reader the wrong mental
   model. The cell-agnostic cut also maps cleanly onto the cell-agnostic
   forwarding signals on MineField.

2. **`pressStart` fires from `mousePressEvent` *before* the reveal/chord
   branch.** Painting has to happen during the hold, not after release —
   Qt delivers mousePressEvent synchronously, so emitting early means the
   event loop repaints the smiley as part of the same user-visible frame.

3. **Right-click-only presses do NOT fire `pressStart`.** Flag cycling is a
   "mark it and move on" gesture; neither reference clones nor users expect
   the header to flicker for a flag. A dedicated unit test
   (`testRightPressDoesNotEmitPressStart`) guards this.

4. **`mouseReleaseEvent` emits `pressEnd` unconditionally.** MainWindow
   tracks tension with a single `m_smileyPressing` bool, so an unmatched
   end (e.g. after a right-click-only press that never fired a start) is a
   harmless no-op. The alternative — tracking in MineButton whether a
   matching start fired — would add stateful bookkeeping for no gain.

5. **`smileyForTensionState()` is a new inline pure helper in `smiley.h`,
   not a modification of `smileyForState()`.** Preserves v1.8.0's contract
   and the existing test cases verbatim. Won/Lost override tension — once
   the game is over the cells are frozen and the indicator should stay on
   its final face; a late mouse-release from mid-click cannot overwrite
   😎/😵. Asserted by `testTensionIgnoredAfterGameOver`.

6. **`MainWindow::setSmileyState()` clears `m_smileyPressing` as part of
   every state transition.** Belt-and-suspenders: if the cell-freeze on
   win/loss intercepts a mouse-release event, there is no way for 😮 to
   stay stranded on the header.

7. **`MainWindow::applySmiley()` is the single point of truth.** Both
   `setSmileyState` and `setSmileyTension` route through it. Makes adding
   any future indicator state (flashing on new record, pulse on first
   click, …) a one-line change rather than a scattered refactor.

8. **`MineField` forwards button signals with `connect(…, signal, …,
   signal)` passthrough.** Avoids writing throw-away slot bodies just to
   re-emit; Qt's signal-to-signal connection is the idiomatic primitive.

**Assumptions:**
- 😮 renders on Apple Color Emoji / Segoe UI Emoji / Noto Color Emoji via
  Qt's default font fallback. Same stack as the already-shipped 🙂/😎/😵,
  so no platform-specific concern.
- Users associate 😮 (face with open mouth) with "holding my breath" /
  tension more strongly than 🫣 (face with hand over mouth). 😮 is the
  closer analogue to the yellow-face Windows Minesweeper indicator; 🫣
  is a more recent 2020+ emoji and renders with a hand that reads as
  peek-a-boo rather than breath-holding.
- No additional telemetry event. The indicator state is derived from
  existing game.started / game.won / game.lost events — counting
  per-press tension flips would bloat the metric without any product
  question it answers.

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
