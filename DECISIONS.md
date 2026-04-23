# Cycle decisions

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
