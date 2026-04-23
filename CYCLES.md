# Autonomous cycles log

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
