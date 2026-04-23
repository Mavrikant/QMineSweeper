# Autonomous cycles log

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
