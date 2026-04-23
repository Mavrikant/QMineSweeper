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
  - Branch: `feat/best-time-date`
  - PR: (pending — see below)
  - Commit: `15bdcad` on `feat/best-time-date`
  - Release tag: `v1.3.0` (pending)
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
