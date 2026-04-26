# Cycle decisions

## 2026-04-26 — Loss-dialog "Average: %1 (best %2)" companion (v1.42.0)

**Chosen:** On the loss dialog, after the existing `Last win: %1`
line, render the lifetime mean winning duration on the current
difficulty as `Average: %1`, with the same `(best %1)` companion
suffix shipped on the win dialog in v1.41 — e.g.
`Average: 1:18.9 (best 0:45.0)`. Gated on `won >= 3 && totalSecondsWon > 0.0`,
the same threshold the win-side encodes via `WinOutcome.winsAfter >= 3`.
**Zero new translatable strings** — both `"Average: %1"` (v1.36) and
`"(best %1)"` (v1.41) keys already exist in every locale and are
reused as-is.

**Why:**
- v1.41.0 cycle log's first listed next candidate ("loss-dialog
  companion to the win-side `Average: %1` line").
- Alternation pattern continues: v1.36 win → v1.37 loss → v1.38 stats
  → v1.39 loss → v1.40 stats → v1.41 win → **v1.42 loss**.
- The win dialog has surfaced the lifetime average + best companion
  for two cycles; the loss dialog now mirrors them so the player has
  the same lifetime-context anchor on losses as on wins. A loss right
  after a stretch of slow wins reads "you survived 0:42, your average
  was 1:18.9 (best 0:45.0)" — the player sees their typical pace and
  best at the moment they need the perspective most.
- Schema-zero. `totalSecondsWon`, `won`, and `bestSeconds` are all
  already persisted, loaded, and tested. The compute is a single
  divide on the loaded `Stats::Record` — no new I/O, no new field on
  `LossOutcome`, no migration.
- Translation-zero. Both `tr()` keys already exist with hand
  translations across all 9 non-en locales; the loss-side render
  reuses them via Qt's automatic context-shared lookup. A previous
  cycle estimate of "~1 new string" turned out to be strictly
  improvable: full reuse drops the cycle's translation cost to zero.

**Rejected alternatives from the v1.41.0 candidate list:**
- *Stats-dialog "Worst" or "Slowest win" column.* Symmetric to Best
  time but needs a new persisted accumulator (slowest counted winning
  duration per difficulty) and a QSettings schema bump. Multi-cycle.
- *Win-dialog flair when current run beats the lifetime average*
  (e.g., "✨ Beat your average!"). Pure flair beat — alternation-
  friendly but breaks the win→loss alternation we're owed this cycle.
  Park for the next win-side beat.
- *Stats-dialog "Median win time" column.* Different statistical
  signal but needs persisting every counted winning duration (not
  just the sum), schema bump. Multi-cycle.

**Rejected naming alternatives:**
- *New translatable key `"Average win: %1"` for the loss-side line
  (more descriptive than the bare "Average: %1" in win-dialog
  context).* Costs 1 new string × 9 locales for marginal clarity —
  the loss-dialog placement directly after `Last win: 4/24/26`
  already establishes the lifetime-win context. Symmetry across the
  two dialogs (same exact line, same exact translation) outweighs
  the "win" qualifier on the loss side.
- *Combined `"Average: %1 (best %2)"` key.* Same trade-off as
  rejected in v1.41 — would mark the v1.36 + v1.41 keys obsolete in
  every `.ts` file and force re-validation of all 18 existing hand
  translations. Two-key approach preserves zero churn.

**Implementation choices:**

1. **Single `Stats::load` per loss.** `MainWindow::onGameLost` already
   loaded the record once (for `priorLastWinDate`); the new fields
   read from the same in-memory `priorRecord` so the loss path stays
   one read regardless of how many lifetime-context lines the dialog
   surfaces.
2. **No `LossOutcome.averageSecondsAfter` mirror field.** Unlike
   `WinOutcome` (which has a pre-recordWin → post-recordWin mutation
   on the relevant fields, so a returned snapshot is meaningfully
   different from a post-call `Stats::load`), `recordLoss` does NOT
   touch `won`, `totalSecondsWon`, or `bestSeconds`. The pre-call
   `Stats::load` and a hypothetical post-call one would return the
   same values for these fields, so threading them through
   `LossOutcome` would be pure ceremony. Compute at the call site
   from `priorRecord` instead.
3. **`won >= 3 && totalSecondsWon > 0.0` compound gate.** Mirrors
   the win-side's `winsAfter >= 3` gate (necessary condition: enough
   wins for an average to be informative) AND the win-side's
   `recordWin` `seconds > 0.0` accumulator gate (defends the divisor
   against the pathological all-sub-tick case where `won == 3` but
   `totalSecondsWon == 0.0`).
4. **Render order: `Streak ended → Last win → Average → (best …)`.**
   The closing arc reads as escalating context: "your streak just
   ended → you've done this before → here's your typical pace →
   here's how close that pace was to your best".
5. **Tests: pin the loaded-record contract, not the dialog text.**
   Six new `tst_stats` tests pin `Stats::load(diff).{won,totalSecondsWon,bestSeconds}`
   under the canonical n=3 case, the n=2-below-gate case, the
   all-sub-tick gate-closed case, the mixed sub-tick + real
   numerator/divisor case, and the post-recordLoss invariance case.
   The dialog rendering itself is presentation-only; the
   loaded-record contract is what a future refactor could break.

## 2026-04-26 — Win-dialog "Average: %1 (best %2)" companion (v1.41.0)

**Chosen:** When the win dialog already shows the `Average: %1` line
(gated at `winsAfter >= 3`), append a parenthetical `" (best %1)"`
suffix rendering the player's lifetime best time on the same
difficulty, e.g. `Average: 1:18.9 (best 0:45.0)`. New translatable key
`"(best %1)"` only — the existing `"Average: %1"` key is preserved
verbatim so v1.36 hand translations continue to apply unchanged.

**Why:**
- v1.40.0 cycle log's first listed next candidate, called out as the
  "smallest informative win-dialog beat".
- Alternation pattern continues: v1.36 win (Average) → v1.37 loss
  (Last win) → v1.38 stats (Last win column) → v1.39 loss (Streak
  ended) → v1.40 stats (Average column) → **v1.41 win** (Average
  companion).
- The Average line by itself answers "how do I usually do?" but not
  "how does this run compare to my best?" The parenthetical anchors
  the average against the player's hall-of-fame for the same
  difficulty, surfacing the gap-from-best inline.
- Schema-zero. `bestSeconds` is already persisted, loaded, and
  hall-of-fame-tested since v1.0; the new `WinOutcome.bestSecondsAfter`
  field reads `r.bestSeconds` post-update (the same value the next
  `Stats::load` will see) so no new I/O at the call site and no
  migration.
- 1 new translatable string × 9 non-en locales — within the cycle
  cap of "≤ 1 string change" set in earlier alternation cycles. All
  9 locales already translate the surrounding `"Average: %1"` line
  and the standalone `"Best time"` column header, so the new
  parenthetical reuses each locale's "best" noun for cross-line
  consistency.

**Rejected alternatives from the v1.40.0 candidate list:**
- *Win-dialog `"Average: %1 — %2 vs your best"` delta variant.* Same
  data, but adds a derived `+25.0s` token rather than a raw best time.
  Two new strings (a sign-prefixed delta plus the tail) instead of
  one, plus the sign-prefix wants per-locale formatting rules
  ("+25.0s" reads as Latin-script across all 10 locales but Arabic
  RTL handling of the leading `+` is fragile). Higher cost; park.
- *Stats-dialog "Wins per day" column.* Needs a new persisted
  accumulator (date of first win for a difficulty + active days) and
  a new QSettings schema. Multi-cycle.
- *Stats-dialog Total row gains an Average footer cell.* Em-dashed
  in v1.40 by design (mixing difficulties of different sizes makes
  the aggregate noise). Would only revisit with a concrete user-
  facing motivator.

**Implementation choices:**

1. **Two tr() strings, not one combined `"Average: %1 (best %2)"`.**
   Keeping `"Average: %1"` unchanged means the v1.36 hand
   translations across all 9 non-en locales continue to apply with
   zero churn — the new key `"(best %1)"` adds exactly one row per
   locale to `apply_translations.py`. A combined key would mark the
   v1.36 key obsolete in every `.ts` file and force re-validation of
   every existing translation.
2. **Always show the suffix when the Average line shows.** No
   `bestSeconds < winAverageSeconds` gate. The sufficient condition
   is `winAverageSeconds > 0.0` which (per `Stats::recordWin`'s
   shared `seconds > 0.0` gate) implies `bestSeconds > 0.0`. Edge
   case `bestSeconds == winAverageSeconds` (every counted win the
   same duration) reads `Average: 20.0 (best 20.0)` and is honest —
   it states consistency rather than hiding the metric.
3. **`WinOutcome.bestSecondsAfter` is the post-update value** — read
   from `r.bestSeconds` after `recordWin`'s newBestTime branch may
   have mutated it. Same convention as `averageSecondsAfter`
   (post-increment / post-update); the win dialog renders the same
   value the next `Stats::load` would return.
4. **No format helper needed.** The suffix is a single
   `tr("(best %1)").arg(formatElapsedTime(bestSeconds))` string.
   Reusing the existing `time_format.h` `formatElapsedTime` keeps
   the inner duration formatting identical to the Average and base
   "You cleared the field in %1." lines.

## 2026-04-26 — Stats dialog "Average" column (v1.40.0)

**Chosen:** Add a 5th column "Average" to the Statistics dialog,
rendering each difficulty's lifetime mean winning duration as
`formatElapsedTime(record.totalSecondsWon / record.won)` (em-dash when
the player has no counted winning duration on that difficulty). Inserted
between "Best time" and "Best (no flag)" so the two time-summary
metrics for each difficulty sit adjacent. Total row shows em-dash for
this column (consistent with the other Best columns, where mixing
difficulties of different sizes makes the aggregate noise rather than
signal).

**Why:**
- v1.39.0 cycle log called for the next stats-dialog beat in the
  alternation pattern (v1.32 loss → v1.33 stats → v1.34 loss → v1.35
  stats → v1.36 win → v1.37 loss → v1.38 stats → v1.39 loss →
  **v1.40 stats**).
- v1.36.0 added `WinOutcome.averageSecondsAfter` and the win-dialog
  `Average: %1` line, but only surfaces it on a winning run *and only*
  on the difficulty just won. The Stats dialog is the natural place to
  compare lifetime average across all three difficulties at one glance,
  without forcing the player to win on each difficulty to surface it.
- Schema-zero. `totalSecondsWon` and `won` already exist and are
  already loaded / saved / tested for the v1.36 win-dialog Average
  line. No new persisted field, no migration, no QSettings key.
- Single new translatable string × 9 non-en locales — within budget.
  All 9 locales already translate the runtime `Average: %1` key, so
  the column header reuses the same noun stem (Ortalama / Promedio /
  Moyenne / Durchschnitt / Среднее / Média / 平均 / औसत / المتوسط)
  for cross-locale consistency.

**Rejected alternatives from the v1.39.0 candidate list:**
- *Win-dialog "Wins so far" tail on Average line, e.g.
  `Average: 1:18.9 (n=12)`.* Same plurals blocker as v1.37: the
  parenthetical wants per-locale pluralisation rules (Russian 3
  forms, Arabic 6) which `apply_translations.py` doesn't yet
  template. Would expand the tooling, not just add a string. Park.
- *Stats-dialog "Best across all" footer cell.* Cell mixes a
  difficulty name and a time; estimated 3 new translatable strings
  plus a tie-break policy decision. Higher cost than this cycle's
  budget.
- *Win % column broken out from Won.* Pure rearrangement; adds zero
  new *information* — Won column already renders `5 (50%)` inline.
- *Win-dialog `🌟 New best streak: %1!` round-number-boundary
  variant.* Pure flair, no new information; park unless a concrete
  user-facing motivator surfaces.

**Implementation choices:**

1. **Standalone formatter header `average_time_format.h`.** Same
   pattern as `time_format.h` / `bv_per_second_format.h` /
   `flag_accuracy_format.h`: small, allocator-free function inlined
   into the Stats dialog and tested directly via `tst_average_time_format`.
   Avoids adding a fourth in-place lambda inside `showStatsDialog`
   (which would join `formatBest`, `formatStreak`, `formatLastWin`,
   `formatBestTimeOrPartial`) and gives the new arithmetic a unit-test
   surface for boundary conditions (won=0, totalSecondsWon=0,
   sub-tick-only wins, division precision).

2. **Em-dash gate on `won == 0 || totalSecondsWon <= 0.0`.** The
   first condition is the divide-by-zero guard; the second handles
   the sub-tick edge case (every counted win was sub-tick → divisor
   is positive but numerator is 0.0, mean is mathematically 0 but
   not informative — surface as "—" instead of "0.0"). Mirrors the
   `WinOutcome.averageSecondsAfter` 0.0 sentinel used by the
   win-dialog Average gate.

3. **No date suffix on the cell.** Best-time / Best-3BV/s / Streak /
   Last-win cells all carry a date because they pin a specific run.
   Average is a lifetime mean — there is no single "average run", so
   the date suffix would be misleading. Plain `formatElapsedTime`
   value, no parenthesised date.

4. **No average column header gate.** The header reads `"Average"`
   even when every cell is em-dash (a brand-new player who has never
   won anything). Mirrors how `Best time`, `Best (no flag)`, etc. all
   render their headers regardless of cell population.

5. **Total row em-dash.** `sum(totalSecondsWon) / sum(won)` is a
   well-defined number, but mixes difficulties of different sizes and
   would mostly reflect whichever difficulty the player plays most.
   Em-dash matches the existing Total-row precedent for Best-time /
   Best-(no flag) / Streak / Best-3BV/s / Best-flag-accuracy / Last-win
   ("doesn't aggregate meaningfully across mixed-size difficulties").

6. **Insert position: column 5 (between "Best time" and "Best (no
   flag)").** Groups the three time-summary metrics together.
   Alternative positions considered: end of table (after "Last win" —
   rejected because Last-win is a date-anchored closing column);
   right after Won (rejected because Best-time → Average → Best-(no
   flag) is the most natural reading order for time-related cells).

7. **Translation strategy.** New lupdate key `"Average"` (column
   header), distinct from the existing `"Average: %1"` (win-dialog
   line). All 9 non-English locales hand-translated reusing the same
   noun stem already shipped for `"Average: %1"`: Ortalama / Promedio /
   Moyenne / Durchschnitt / Среднее / Média / 平均 / औसत / المتوسط.

**Backwards-compat behaviour:** entirely additive. Pre-1.36 plists
without `total_seconds_won` load that field as 0.0 (existing
`testLegacyRecordWithoutTotalSecondsWonLoadsAsZero` test pins this),
so legacy users see "—" for Average until their next win — the same
clean-slate behaviour as v1.36 / v1.37 introduced for Average-line
and Last-win-line gates.

## 2026-04-26 — Loss dialog "💔 Streak ended at %1" line (v1.39.0)

**Chosen:** Add a recap line `💔 Streak ended at %1` to the loss
dialog, surfaced when this loss broke an active winning streak of 2 or
more on this difficulty. The streak length is captured by
`Stats::recordLoss` *before* it zeros `currentStreak` and returned via
the existing `LossOutcome` struct as a new `priorStreak` field.

**Why:**
- v1.38.0 cycle log explicitly listed alternation pattern continuing
  with a loss-dialog beat. Loss-side has been getting recap lines and
  flairs since v1.22 but never surfaced *what just got broken* —
  the win-side has had `🔥 Streak: %1` and `🌟 New best streak: %1!`
  since v1.18, but losing on a 5-streak silently zeroed the field with
  no acknowledgement.
- Player-visible new information. Reframes the loss as the cost of a
  fall, not a baseline state. Pairs with the v1.37.0 `Last win: %1`
  line to give a two-beat closing arc on the loss dialog: "💔 Streak
  ended at 5" (you had momentum) followed by "Last win: 25.04.2026"
  (and you've done this before).
- Schema-zero. `currentStreak` already exists and is already loaded /
  saved / tested for the Stats dialog "Streak" column. No new persisted
  field, no migration, no QSettings key.
- Single new translatable string × 9 non-en locales — within budget.

**Rejected alternatives from the v1.38.0 candidate list:**
- *Loss-dialog "X days ago" follow-up to v1.37 Last win.* Same blocker
  as the v1.37 cycle: requires per-locale plural rules (Arabic has 6
  plural forms, Russian 3) which `apply_translations.py` doesn't yet
  template. Would expand the tooling, not just add a string. Park.
- *Win % column broken out from Won.* Pure rearrangement, adds zero new
  *information* — Won column already renders `5 (50%)` inline. Park.
- *Win-dialog "Wins so far" tail on Average line.* Same plurals
  blocker. Park.
- *Stats-dialog "Best across all" footer cell.* Cell mixes a difficulty
  name and a time; estimated 3 new translatable strings plus a
  tie-break policy decision. Higher cost than this cycle's budget; the
  loss-side beat is more user-visible at the same surface area.

**Implementation choices:**

1. **Capture in `recordLoss`, return via `LossOutcome`.** Mirrors how
   `WinOutcome.currentStreak` / `newBestStreak` are returned to the
   win-side caller. Avoids the alternative of double-loading the record
   in `MainWindow::onGameLost` — one `Stats::load(diffName)` already
   happens for `priorLastWinDate`, and slipping a second load just for
   the streak would be needless I/O on the same QSettings tree.

2. **Gate `>= 2` on display, not on the field.** The field always
   reports the raw value so future callers (telemetry, tests, or a
   sortable Stats-dialog column) can read it without a separate code
   path. The dialog gate `>= 2` matches the existing win-side
   `🔥 Streak: %1` gate for symmetry: a single win that just got
   broken isn't a streak worth mourning.

3. **Recap line, not a flair prepend.** The loss dialog already has
   two flair prepends (`🎯 New best %!`, `🚩 New best flag accuracy!`)
   — both *positive* hall-of-fame achievements. Putting a *negative*
   "Streak ended" alongside them as a flair would jar visually.
   Instead, place it as a recap line between "Question marks: %1" and
   "Last win: %1" so the loss narrative reads top-to-bottom with the
   broken-streak fact alongside the other recap lines, then closes on
   the historical anchor.

4. **Replays / customs hide the line.** They don't call
   `Stats::recordLoss`, so `lossOutcome.priorStreak` stays at the
   default-constructed `0` and the gate hides the line. By design — a
   custom-board loss doesn't actually break the standard-difficulty
   streak (and a replay loss is a re-run of an already-counted layout).

5. **Translation strategy.** New lupdate key
   `"💔 Streak ended at %1"`. All 9 non-English locales hand-translated
   reusing the existing `🔥 Streak: %1` / `🌟 New best streak: %1!`
   noun stems (Seri / Racha / Série / Serie / Серия / Sequência /
   连胜 / लगातार जीत / سلسلة الفوز) for cross-line consistency. No
   churn on existing keys — `lupdate` reports `1 new and 109 already
   existing` per locale, confirming zero accidental drift.

**Backwards-compat behaviour:** entirely additive. Pre-1.39 plists
read as before; existing `streak_current` field is the only thing
read, and it's been persisted since v1.18. No new QSettings key, no
migration, no regression risk on legacy data.

## 2026-04-26 — Stats dialog "Last win" column (v1.38.0)

**Chosen:** Add a 9th column "Last win" to the Statistics dialog,
rendering each difficulty's `Stats::Record::lastWinDate` via
`QLocale::ShortFormat` (em-dash when the player has never won).

**Why:** v1.37.0 surfaced the `lastWinDate` field as a single line
on the loss dialog — but only one difficulty at a time. The Stats
dialog is the natural place to compare the metric across all three
difficulties at one glance. It's also pure presentation: the field
already exists and is already loaded/saved/tested by the v1.37.0
schema work, so the column is a passive reader with zero new state
and zero migration risk. Continues the loss-dialog → stats-dialog
alternation pattern (v1.32→v1.33→v1.34→v1.35→v1.37→**v1.38**).

**Rejected alternatives:**
- *Win % column broken out from Won.* Pure presentation but adds
  zero new *information* — the Won column already renders `5 (50%)`
  inline. v1.38 spends its budget on a column that surfaces a field
  that's currently invisible from this dialog.
- *Win-dialog "Wins so far" tail on the Average line* (e.g. "Average:
  1:18.9 (n=12)"). Useful denominator context but blocked on
  translating the parenthetical cleanly across all 10 locales
  (Arabic / Hindi numeral systems, Russian plural rules — adds
  tooling work, not a 1-string add).
- *"Best across all" footer for Best time.* Cell would mix a
  difficulty name and a time — estimated 3 new translatable strings
  plus a tie-break policy decision. Higher cost than v1.38's budget.
- *Replacing the Total row's em-dash for Last win with the most-recent
  per-row date.* Considered, rejected — would shadow whichever
  per-row cell is most recent. Duplicate signal, no new information.
  Pinned in the mainwindow.cpp Total-row comment.

**Translation policy:** new lupdate key "Last win" (column header,
no `%1` placeholder) is distinct from the existing v1.37.0 key
"Last win: %1" (verb-phrase loss-dialog line). Each non-English
translation is the existing v1.37.0 translation with the `: %1`
suffix stripped. No translation churn on existing keys.

**Backwards-compat behaviour:** unchanged from v1.37.0. Pre-1.37
plists with `won > 0` but no `last_win_date` key load the date as
invalid → cell renders em-dash until the player's next 1.37+ win.
Clean-slate seeding by design (best-date is the date of the
*fastest* run, not the most recent — back-filling could lie by
months).

## 2026-04-26 — Loss dialog: "Last win: %1" line when difficulty has a prior win (v1.37.0)

**Chosen:** Add a `Last win: %1` line to the loss dialog that shows
the calendar date of the player's most recent win on the current
difficulty, when one exists. Persist `lastWinDate` on
`Stats::Record` (separate from `bestDate`, which only stamps the
current best-time run). `Stats::recordWin` always overwrites
`lastWinDate` with `onDate` (every counted win, not just record
runs). The loss dialog's `MainWindow::onGameLost` reads the
per-difficulty record and renders the line — gated on
`lastWinDate.isValid()` — independent of whether the loss itself
was counted (replays / customs render the line too if the standard
difficulty has a prior win).

**Why this one:**
- v1.36.0 cycle log explicitly listed "Loss-dialog 'Time since last
  win' line" as the top next candidate; previously parked from
  cycles 28–32 in favour of leaner-surface picks. The win dialog
  just gained a line in v1.36; alternation pattern (v1.32 loss →
  v1.33 stats → v1.34 loss → v1.35 stats → v1.36 win) asks for a
  loss-dialog beat next.
- New player-facing information: the loss dialog already carries
  six lines and four flairs, but none of them anchor the loss to
  the player's broader history. "Last win: 25.04.2026" reframes a
  loss as a stumble rather than a streak — psychological nudge for
  players on a long losing streak.
- Schema diff is well-trodden in this codebase (seven prior cycles
  have added a persisted `Stats::Record` field with
  load/save/reset/test churn): ~50 LOC of production diff plus
  tests, one new translatable string × 9 locales.

**Rejected alternatives from the v1.36.0 candidate list:**
- *Stats-dialog "Best across all" footer cell.* Pure presentation
  but the cell value mixes a difficulty name and a time; the
  v1.35.0 / v1.36.0 cycle logs estimated 3 new translatable
  strings. Park.
- *Win % column broken out from Won.* Pure presentation, ~30 LOC,
  1 new translatable string. Smallest possible diff but adds zero
  *new* information — the Won column already renders `5 (50%)`
  inline; pulling it into its own column is rearrangement, not
  signal. Park.
- *Win-dialog "Wins so far" tail* on the Average line, e.g.
  `Average: 1:18.9 (n=12)`. Surfaces useful denominator context
  but the parenthetical translates awkwardly across all 10 locales
  (Arabic and Hindi numeral systems, Russian plural rules). Park.

**Implementation choices:**

1. **Show a date, not a duration.** "Last win: 25.04.2026" instead
   of "Time since last win: 2 days". Avoids plural-forms hell
   (Arabic alone has 6 plural forms; "2 day(s) ago" requires Qt's
   `%n` machinery and a per-locale plural table that
   `apply_translations.py` would need to grow). The date format
   reuses `QLocale().toString(date, QLocale::ShortFormat)`, which
   the Stats dialog has rendered consistently since v1.3.
   Trade-off: less emotional resonance ("3 days ago" reads more
   visceral than "23.04.2026"), but recoverable in a future cycle
   if telemetry / feedback shows the date is too cold.

2. **`recordWin` overwrites `lastWinDate` unconditionally.**
   Mirrors the design of `bestDate` (date of the best-time run)
   but tracks the *most recent* win instead of the *fastest*. The
   two stamps are independent: a new win always updates
   `lastWinDate`; only a strictly-faster win updates `bestDate`.
   Keeps the persistence layer dumb — display policy lives at the
   call site.

3. **Threshold = "lastWinDate is valid".** No min-wins gate. The
   first win on each difficulty seeds `lastWinDate`, and every
   subsequent loss surfaces it. No equivalent of the win-dialog's
   `≥ 3 wins` because the line is informational about the *date*,
   not statistical about a *distribution* — n=1 is a perfectly
   valid date.

4. **Loss-dialog rendering is unconditional on replay/custom
   status.** The line reflects the per-difficulty record, not the
   current run. A replay-loss on Beginner where the player has
   previously won Beginner shows "Last win: …" — the data is
   correct regardless of whether *this* loss was counted. Custom
   games don't have stats, so `Stats::load("Custom")` returns the
   default record with an invalid `lastWinDate`; the line stays
   hidden by the `isValid()` gate. No special-casing.

5. **Backwards-compatible load.** A pre-1.37 plist with no
   `last_win_date` key reads as the default-constructed (invalid)
   `QDate`, so the line stays hidden until the player's *next* win
   in 1.37+. Considered seeding `lastWinDate = bestDate` on first
   load — rejected because best-date is the date of the best-time
   run, which may have been months ago even if the player won
   yesterday. Clean-slate seeding is the honest default.

6. **Reset semantics.** `Stats::reset(name)` removes the
   `last_win_date` key alongside the other per-record keys.
   `resetAll()` already wipes the whole `stats/` group.

7. **No `WinOutcome` field for `lastWinDate`.** Unlike v1.36's
   averaging path, the loss dialog needs the *previous* lastWinDate
   (before this run), not the post-update one. The dialog reads
   `Stats::load(diffName)` directly inside `onGameLost` — there's
   no `WinOutcome`-equivalent on `recordLoss` that needs to carry
   it, and threading it through `recordLoss`'s return type would be
   wrong (the loss didn't update it).

**Assumptions:**
- Showing a bare date is the right rendering (decision 1).
- The line should be unconditional on replay/custom (decision 4).
- The line should NOT show on the *win* dialog. Its job is to
  contextualise a loss; on a win the player has just *demonstrated*
  the most recent win, so the line collapses to "Last win: today",
  which is noise.
- Format reuses `QLocale().toString(date, QLocale::ShortFormat)` —
  same locale-aware date format the Stats dialog uses for all four
  best-* date stamps. Zero new format helpers.

## 2026-04-26 — Win dialog: "Average: %1" line after ≥3 wins (v1.36.0)

**Chosen:** Add an `Average: %1` line to the win dialog that shows the
player's mean winning time on the current difficulty once they have
won that difficulty at least 3 times. Persist `total_seconds_won`
on `Stats::Record` (sum of every counted winning duration), accumulate
inside `Stats::recordWin`, and surface the post-update wins-count and
mean via two new `Stats::WinOutcome` fields so `MainWindow` can render
the line without re-loading.

**Why this one:**
- v1.35.0 cycle log explicitly listed "Win-dialog Average time after
  ≥3 wins" as a next candidate; alternation pattern of recent cycles
  (v1.32 loss-dialog → v1.33 stats → v1.34 loss-dialog → v1.35 stats)
  asks for a win/loss-dialog beat next.
- New player-visible information that isn't already shown anywhere
  else in the app: best time tracks the *peak* run, average tracks
  the *typical* run — the lever a player wants to move with practice.
  Mirrors how speedrun trackers split "PB" from "average of last N".
- Schema diff is well-trodden in this codebase (six prior cycles have
  added a persisted field on `Stats::Record`); ~50 LOC of production
  diff plus tests, one new translatable string × 9 locales.

**Rejected alternatives from the v1.35.0 candidate list:**
- *Loss-dialog "Time since last win" line.* Also requires a new
  `Stats::Record` field (`last_win_date`) and a translatable string;
  comparable cost. Picked the win-side because the win dialog is a
  leaner surface than the v1.32–v1.35-built-up loss dialog (six
  lines and four flairs already), so adding a new line there
  delivers more incremental signal. Park "Time since last win"
  for the next cycle.
- *Stats-dialog "Best across all" footer cell.* Pure presentation
  but the cell value mixes a difficulty name and a time; the
  v1.35.0 cycle log estimated 3 new translatable strings. Park.
- *Win % column broken out from Won.* Pure presentation, ~30 LOC,
  1 new translatable string. Smallest possible diff but adds zero
  new *information* — the Won column already renders `5 (50%)`
  inline. Park.

**Implementation choices:**

1. **Threshold ≥ 3 wins.** A 1-win average equals best time exactly
   (trivially); a 2-win average is a single data point of variation
   and feels gimmicky. ≥ 3 keeps the line meaningful — the same
   threshold standard speedrun trackers use ("ao3" / average of 3).
   The threshold is checked at the call site (`MainWindow`), not in
   `Stats`, so the persistence layer stays uncoupled from display
   policy.

2. **`total_seconds_won` only accumulates on `seconds > 0` wins.**
   Mirrors the existing `bestSeconds` gate. A test-only sub-tick win
   (`seconds == 0.0`) doesn't poison the divisor. The accumulator's
   sentinel is the same as `bestSeconds`: 0.0 == "no recorded
   winning duration".

3. **`WinOutcome` carries `winsAfter` + `averageSecondsAfter`, not
   the raw accumulator.** The caller doesn't need to know the
   accumulator exists — it only wants the average. `winsAfter`
   doubles as the threshold gate. Keeps `MainWindow` from having
   to re-`load()` the record after `recordWin`.

4. **Replays and custom games are excluded.** `Stats::recordWin` is
   already gated on `!m_isReplay && !m_isCustom` in
   `MainWindow::onGameWon`; nothing new needed. Memorising a board
   to game the average would otherwise be a trivial cheat.

5. **Backwards-compatible load.** A pre-1.36 plist with no
   `total_seconds_won` key reads as 0.0. The first 1.36+ win on
   each difficulty seeds the accumulator with that win's duration —
   the Average line will read identical to the win's clock until
   the third win. Considered (and rejected) seeding from
   `bestSeconds × won` on first load: best is the *fastest* run,
   not the average, so the first displayed average would be
   optimistically wrong. The clean-slate approach makes the first
   three displayed averages drift slightly until the accumulator
   catches up — acceptable.

6. **Reset semantics.** `Stats::reset(name)` removes the
   `total_seconds_won` key alongside `played` / `won`. `resetAll()`
   already wipes the whole `stats/` group.

**Assumptions:**
- ≥ 3 wins is the right threshold (decision 1).
- Showing the Average line *every* win past the threshold is fine.
  Considered hiding it on a new-best-time run so the 🏆 line isn't
  crowded; rejected because the average is always *higher* than
  best (best = minimum), so seeing the gap is itself informative.
- Format reuses `formatElapsedTime` so the average reads with the
  same M:SS.S / H:MM:SS.S grammar as every other time string —
  zero new format helpers, zero new translatable formats.

## 2026-04-26 — Statistics dialog: Total row aggregating Played / Won (v1.35.0)

**Chosen:** Add a 4th, bold-styled `Total` row at the bottom of the
Statistics table that sums `Played` and `Won` across the three
difficulties; show aggregate win % the same way per-difficulty rows do;
collapse all per-best columns to em-dash. Pure presentation, no
QSettings schema change.

**Why this one:**
- v1.34.0 cycle log explicitly tagged this as the lead "Stats-dialog
  row totals" candidate ("pure presentation aggregation; no new
  persistence"). Smallest viable scope of the three named candidates,
  and the only one with zero schema risk.
- Concrete user value: surfaces the lifetime session footprint
  without making the user mentally add three numbers.
- Exhausts a known UX gap with a ~30-LOC diff and 1 new translatable
  string × 9 locales — well under the cycle budget.

**Rejected alternatives:**
- *"Best across all difficulties" cell* in the Best time column. The
  smallest-board (Beginner) best would always win, which is
  misleading and redundant with the Beginner row already showing it.
  Em-dash is the honest call.
- *Background colour for the Total row.* Risks dark-mode contrast
  issues; bold uses the same palette token and reads well in any
  theme.
- *Total row at the top.* Spreadsheet convention puts sum rows after
  the data, and the existing Beginner / Intermediate / Expert
  ordering is left-anchored to the smallest difficulty.
- *Loss-dialog "Time since last win" line.* Higher-effort candidate
  from the v1.34.0 list; requires new `last_win_date` QSettings
  field. Park.
- *Win-dialog "Average time" line.* Same — requires
  `total_seconds` divisor in persistence. Park.

**Assumptions:**
- `std::uint64_t` accumulators preempt the (physically unreachable
  but theoretically possible) `uint32` overflow when summing three
  Played counts.
- Integer-percent rounding for the aggregate win % matches
  per-difficulty row formatting.
- A bold font on each `QTableWidgetItem` (via `setFont(boldFont)`)
  is sufficient visual differentiation; no need for a separator row
  or a layout-band footer.
- Reset confirmation copy needs no update — already says "all
  records".
- The new `Total` translatable key is short enough that all 9
  hand translations fit in the existing Difficulty column width
  set by the longer `Intermediate` / `Başlangıç` etc. strings.

**Translation cost:** 1 new key × 9 non-English locales (TR `Toplam`,
ES `Total`, FR `Total`, DE `Gesamt`, RU `Итого`, PT `Total`,
ZH `总计`, HI `कुल`, AR `المجموع`). 50/50 coverage preserved.

**Skipped:**
- *Per-best column aggregation.* See Rejected alternatives.
- *Win % broken into its own column.* Cleaner but adds a 9th column;
  defer to a future cycle.

## 2026-04-25 — Loss dialog: 🚩 New best flag accuracy! flair (v1.34.0)

**Chosen:** Mirror v1.31.0 → v1.34.0 the same way v1.30 was followed
by v1.31: v1.33.0 added a "Best flag accuracy" Stats column backed
by `LossOutcome.newBestFlagAccuracyPercent`, but the bool was only
consumed by telemetry. This cycle prepends a celebratory
`🚩 New best flag accuracy!` line on the loss dialog when the bool
fires — the per-game flair counterpart to the lifetime column.

**Why this and not something else:**
- It was the lead "Next candidate" in the v1.33.0 cycle log —
  "persistence layer already returns the bool; only the dialog needs
  to consume it." Smallest and highest-leverage diff in the queue.
- Closes the same per-loss-readout → lifetime persistence → flair
  triplet the project has now run twice (v1.30 ⚡ flair → v1.31
  Best 3BV/s column was the win-side; v1.32 Correct flags line →
  v1.33 Best flag accuracy column → v1.34 🚩 flair is the loss-side).
- Pure additive: no schema change, no new persistence, no new
  computation. ~10 LOC of production diff plus 1 translatable
  string × 9 locales.

**Why 🚩 and not 🏹:**
- 🚩 (red flag) maps directly to the in-game flag mechanic — the
  same visual the player has been planting on cells throughout the
  game. Recognised at a glance.
- Distinct from the v1.29 🎯 (safe-percent) flair on the same
  dialog, so a loss that improves both records reads with two
  unambiguous icons. The v1.33 cycle log explicitly flagged this
  ambiguity as the blocker — "Two flairs on the same emoji is the
  blocker — pick a different emoji (`🎯 → 🏹`?) or a longer
  distinguishing label." 🚩 is more thematic than 🏹 (no
  archery in Minesweeper) and the project doesn't use it anywhere
  else (verified by grep).

**Why prepend AFTER 🎯 (so 🚩 ends up leftmost when both fire):**
- Mirrors the win-side ordering convention — `⚡` (the flair shipped
  most recently in v1.30) is prepended last and ends up leftmost,
  ahead of `🌟/🔥` (v1.18 streak) and `🏆` (v1.2 best time).
- Same logic on the loss side: `🚩` (this cycle) is the newest flair,
  so it goes to the left of `🎯` (v1.29). Reading left-to-right the
  player sees the freshest celebration first.

**Why no new test:**
- The bool's source — `Stats::recordLoss(...).newBestFlagAccuracyPercent`
  — was exhaustively tested in v1.33.0 (16 new tests in
  `tst_stats.cpp` covering strict-greater-than semantics, the
  `flagsPlaced > 0` gate, replay/custom exclusion, the legacy-plist
  zero-load, the safePercent independence matrix, and the
  `operator bool` backwards-compat pin).
- The new flair is `if (lossNewBestFlagAccuracy) text.prepend(...)`
  inside `MainWindow::showEndDialog` — pure boolean propagation, not
  unit-testable without a `QMessageBox` harness, and consistent with
  prior dialog-flair cycles (v1.18 streak, v1.29 🎯, v1.30 ⚡)
  which also added flairs without dialog tests.
- Both `showEndDialog` call sites updated; the win-side passes
  `false` for the new bool (a win never sets a flag-accuracy record;
  see v1.33 `testRecordWinDoesNotTouchBestFlagAccuracy`).

**Translation cost:** 1 new key × 9 locales, all hand-translated.
Mirrors the column-header phrasing already established in v1.33:
e.g. tr_TR "Best flag accuracy" → "En iyi bayrak isabeti", flair
becomes "🚩 Yeni en iyi bayrak isabeti!". 50/50 translation
coverage preserved.

## 2026-04-25 — Loss dialog: "Correct flags: %1 / %2" line (v1.32.0)

**Chosen:** Add a flag-accuracy readout to the loss dialog. Right after
the existing `Flags placed: %1` line, a new `Correct flags: %1 / %2`
line shows how many of the user's placed flags landed on actual mines
at the moment of explosion. Form mirrors the existing
`Partial 3BV: X / Y · 3BV/s: Z` pair-line. Gated on the same
`flagsPlaced > 0` predicate as the companion line so a no-flag boom
stays clean. Backed by a new `MineField::correctFlagsPlaced()` walking
live cells for `isFlagged() && isMined()`.

**Why this and not something else:**
- Continues the v1.21.0–v1.27.0 thread of small loss-dialog detail
  additions. The v1.24.0 cycle introduced `Flags placed: %1` but
  stopped short of accuracy — the missing companion is a natural fit.
- High signal-to-effort: ~30 LOC of source, +1 translatable string ×
  9 locales, +7 test cases. Pure addition (no existing key changes,
  no schema changes, no new QSettings keys).
- Useful gameplay feedback. Did your flagging discipline hold up, or
  were you guessing? The total-flags line alone can't tell you.
- Matches user instinct: the `flagsPlaced` doc comment explicitly
  notes "the loss path does not auto-flag, so reading this on a loss
  gives the user's actual flag count at the moment of explosion" —
  pairing that with correctness is the obvious next step.

**Implementation invariants:**
- `correctFlagsPlaced() ≤ flagsPlaced()` — the field can never count
  a flag that isn't placed. Encoded as test cases (all-on-mines,
  none-on-mines, mixed).
- A `?` on a mined cell does NOT count as a correct flag — the
  marker is `Question`, not `Flag`. Regression-guarded by
  `testCorrectFlagsPlacedQuestionDoesNotCount` so any future change
  that conflates marker states gets caught.
- The walk runs only at end-of-game (called once from
  `MainWindow::onGameLost`), so the O(rows × cols) ≤ 480 cost is
  acceptable. Same pattern as `questionMarksPlaced()`.
- `revealAllMines()` does not touch `m_marker`, so the count survives
  the loss reveal — guarded by `testCorrectFlagsPlacedPreservedOnLoss`.

**Why a separate line, not `Flags placed: 8 (correct: 6)`:**
- Combining would change the existing translation key for
  `Flags placed: %1`, forcing re-translation across 9 locales.
- Two clean lines (`Flags placed: 8` + `Correct flags: 6 / 8`) read
  more naturally than a parenthetical and match the
  `Partial 3BV: X / Y · 3BV/s: Z` precedent for paired stats.

**Why not "Flag accuracy: 75 %":**
- Loses absolute counts. With 1–2 placed flags, a percentage reads
  awkwardly (50 % from 1/2 vs 50 % from 4/8 are very different).
- The `correct / total` form matches `Partial 3BV: %1 / %2` and
  avoids introducing a fourth percent sign on the loss dialog
  (already has `You cleared %1% of the board.`).

**Telemetry:** `game.lost` event gains a `correct_flags` tag
alongside the existing `flags`/`qmarks`. Useful for future analysis
of flag-accuracy distributions across difficulties.

**Tests added (7):**
- `testCorrectFlagsPlacedZeroBeforeAnyFlag` — empty initial state.
- `testCorrectFlagsPlacedCountsOnlyFlagsOnMines` — mixed (1/2).
- `testCorrectFlagsPlacedAllOnMines` — boundary (2/2).
- `testCorrectFlagsPlacedNoneOnMines` — boundary (0/2).
- `testCorrectFlagsPlacedQuestionDoesNotCount` — regression guard:
  marker cycle through Question must not inflate the counter.
- `testCorrectFlagsPlacedPreservedOnLoss` — revealAllMines must not
  disturb flag state.
- `testCorrectFlagsPlacedResetByNewGame` — newGame clears prior
  cells.

## 2026-04-25 — Stats dialog: Best 3BV/s column (v1.31.0)

**Chosen:** Add a 7th column "Best 3BV/s" to the Statistics dialog
table — the lifetime hall-of-fame counterpart to v1.30's
`⚡ New best 3BV/s!` win-dialog flair. Reads the per-difficulty
`bestBvPerSecond` + `bestBvPerSecondDate` fields shipped in v1.30 and
renders them via a new pure helper (`bv_per_second_format.h`).

**Why this and not something else:**
- The v1.30 cycle log explicitly tagged this as the highest-value next
  cycle filler ("the lifetime hall-of-fame counterpart to today's
  flair … reads the just-shipped `bestBvPerSecond` + `bestBvPerSecondDate`
  fields, renders as a new 7th column on the Stats table. ~50 LOC + 1
  translatable column header × 9 locales").
- The persistence layer is already in place — no schema change, no new
  QSettings keys, no new state. Pure presentation.
- Same cycle-shape as v1.27 → v1.28 (per-loss partial 3BV → lifetime
  hall-of-fame for the same metric). Closes the win-side mirror of
  that thread.

**Render format:**
- `bvPerSecond <= 0` → `"—"` (no record yet — same em-dash sentinel
  the Best-time / Best-(no flag) cells already use).
- `bvPerSecond > 0`, no date → `"X.YZ"`.
- `bvPerSecond > 0`, date → `"X.YZ  (LL.LL.YYYY)"`.
- Two-decimal rate matches the live win-dialog
  `"3BV: %1 · 3BV/s: %2"` format and the `⚡ New best 3BV/s!` flair, so
  the persisted lifetime record reads identically to the celebration
  the user just saw on their best run.
- Locale-formatted date inlined in parentheses with two-space
  separator — mirrors Best-time / Best-(no flag) / Streak cells. No
  new column header per stat, no extra translatable string for the
  date format.

**Why a free helper, not a private lambda:**
- The Stats dialog itself isn't unit-tested (it lives behind
  `QDialog::exec()`); past stats-dialog cycles (v1.28, v1.29, v1.30)
  tested only the persistence layer. To get tests on the new
  presentation logic without standing up a `tst_mainwindow`, the
  formatter is extracted to `bv_per_second_format.h` analogous to
  `time_format.h` and tested in `tst_bv_per_second_format.cpp`.
- Inline-header `formatBvPerSecondCell(double, const QDate&)` keeps
  the call site identical to the prior lambda-based pattern.

**Why "Best" not "Highest" / "Top" in the header:**
- Matches existing column terminology ("Best time", "Best (no flag)").
  Per-locale, the same word-for-"best" used in `Best (no flag)` is
  reused — except German, where `Bestzeit` is specifically
  best-*time* and would mislead on a rate; switched to `Bestwert 3BV/s`,
  which also matches v1.30's German flair (`Neuer Bestwert 3BV/s!`).

**Strict-greater-than semantics inherited:**
- The new cell only displays the existing `bestBvPerSecond` field;
  the strict-greater-than gate on `recordWin` (pinned in v1.30 by
  `testRecordWinWithEqualBvRateKeepsOriginalDate`) governs whether
  the field updates. No new gate added in this cycle.

**Replay/custom exclusion already covered:**
- `recordWin` is only called from the `!excludedFromStats` branch in
  `MainWindow::onGameWon`. Replay and Custom wins never touch
  `bestBvPerSecond`, so the new cell can never display a memorised-
  board record. Inherited from v1.13.0 / v1.30.0.

**Tests added:**
- 11 cases in `tst_bv_per_second_format.cpp`:
  zero/negative/NaN→`"—"`; +inf documented (returns `"inf"` from
  printf, non-empty string asserted); positive without date renders
  two decimals (`1.234→"1.23"`, `0.5→"0.50"`, `12.0→"12.00"`);
  positive with date appends locale short date with two-space
  separator; trailing-zero preserved (`2.10→"2.10"`); rounding
  (`1.236→"1.24"`, `1.234→"1.23"`); very small positive
  (`0.005→"0.01"`); empty-ISO-string-derived invalid date drops
  parenthesised suffix (the persistence-layer load path for
  no-record-yet).
- Adversarial mutation runs verified by mutating the em-dash
  sentinel (3 tests fail) and `%.2f` → `%.1f` (6 tests fail). The
  tests catch the bugs they're supposed to.

## 2026-04-25 — Win dialog: ⚡ New best 3BV/s flair (v1.30.0)

**Chosen:** Add a per-difficulty lifetime `bestBvPerSecond` (double) +
`bestBvPerSecondDate` (QDate) pair to `Stats::Record`, and on a win
that strictly beats the prior rate, surface a `⚡ New best 3BV/s!`
flair on the win dialog. Mirrors the existing `🏆 New record!` flair
gate but for the canonical efficiency metric (3BV/s) instead of pure
clock time.

**Why this and not something else:**
- The cycle 24, 25, 26 logs all explicitly re-parked this as the
  next-highest-value win-side feature. Three deferrals is enough — the
  persistence-layer cost is fixed (two new keys, two new fields) and
  small enough to fit a single cycle.
- 3BV/s is the canonical Minesweeper efficiency metric (already
  surfaced on every win dialog since v1.14.0 as the "3BV: %1 ·
  3BV/s: %2" line). A new personal best at 3BV/s is the speedrun
  community's primary progress signal — much more meaningful than a
  faster clock time on a lower-3BV (i.e. easier) board.
- The win dialog already has three flair slots (`🏆`, `🌟`, `🔥`,
  plus the `🏃` no-flag prefix). Adding `⚡` for "New best 3BV/s!"
  completes the speedrun-recognition picture in one micro-feature.

**Independence from the existing `🏆 New record!` flair:**
- Best-time and best-3BV/s are independent records. A faster win on a
  smaller board could set a new best clock without touching 3BV/s
  (lower 3BV → lower 3BV/s); a slower win on a denser board could set
  a new best 3BV/s without beating the clock. The cycle's tests pin
  both axes independently.
- Both flairs can fire on the same dialog. The render order (top to
  bottom): `⚡` → `🌟`/`🔥` → `🏃` → `🏆`. Pre-prepended bottom-up so
  the user reads the strongest result (`🏆 New record!`) first, then
  the modifiers. Pinned by `WinOutcome` field ordering and the
  prepend-only `showEndDialog` path.

**Strict greater-than semantics:**
- A tie does NOT bump the date or fire the flair. Mirrors the
  best-streak / best-percent-cleared convention pinned in cycles 22
  and 25. Floating-point rounding makes ties effectively impossible
  in practice (3BV/s is double-precision), but the semantics matter
  for the rare deterministic case (e.g. a fixed-layout test, a
  pathological setFixedLayout-with-known-bv run).
- Persistence layer guards against `bvPerSecond <= 0.0` — a
  sub-tick win (only reachable from `setFixedLayout`-driven test
  setups; the live timer always advances at least 0.1s) returns
  `bvRate == 0.0` and skips the update path entirely. Mirror of the
  `recordWin` zero-seconds sentinel.

**Replay/custom exclusion (already in place):**
- The new path runs inside `MainWindow::onGameWon`'s existing
  `!excludedFromStats` branch (replay or custom games skip
  `recordWin` entirely), so a memorised-board run can never set a
  new lifetime best 3BV/s. No new gate needed.

**API shape:**
- New parameter on `recordWin`: `double bvPerSecond = 0.0` (trailing
  default, source-compat for existing callers and 17 test sites).
- `WinOutcome` gains `bool newBestBvPerSecond{false}` (additive
  field; existing readers of `newRecord` / `currentStreak` /
  `newBestStreak` are unaffected).
- `MainWindow::showEndDialog` gains a 16th positional parameter
  `bool winNewBestBvPerSecond`. The 14-then-15 parameter pattern
  established in v1.28.0 / v1.29.0 stays — a struct refactor would
  be churn for no net benefit at this size.

**Why a new flair string and not reuse `🏆 New record!`:**
- The existing flair semantically means "fastest clock time ever".
  Reusing it for "best 3BV/s" would conflate two different records
  and break the user's intuition built up since v1.3.0. A distinct
  glyph (`⚡` for speed/electricity, recognisable as efficiency) +
  distinct copy (`New best 3BV/s!`) keeps the records visually
  separated.

**Telemetry:**
- Add `new_best_bv_per_second` boolean tag on `game.won`. Mirrors
  v1.29.0's `new_best_safe_percent` tag on `game.lost`. Lets us see
  the 3BV/s record-bump frequency without inspecting individual
  payloads.

**Skipped (parked for next cycle):**
- *Stats-dialog "Best 3BV/s" column.* The Stats-side counterpart to
  this flair. Bundle into a near-future cycle to amortise the column
  header translation cost (10 locales × 1 string).
- *Loss-dialog "Time since last win" line.* Needs a new
  `last_win_date` field (separate from `bestDate`, which is the date
  of the best-time win, not the most recent). Defer to a future
  cycle.
- *Reset of `bestBvPerSecond` on a stats schema migration.* Not
  needed — additive QSettings keys load as 0.0 / invalid date for
  upgrading users; the first 1.30.0 win seeds the record naturally.

## 2026-04-25 — Loss dialog: partial-3BV line (v1.27.0)

**Chosen:** Replace v1.25.0's static `Board 3BV: %1` line on the loss
dialog with the speedrun-canonical `Partial 3BV: X / Y · 3BV/s: Z` —
strict superset where X is how many of the board's 3BV "clicks" the
player effectively cleared at the moment of explosion, Y is the total
board 3BV (= the old line's value), Z is X / elapsed = the player's
per-second clearing pace at death. Implement via a new
`MineField::partialBoardValue()` accessor that re-walks
`compute3BV()`'s region/isolated-cell partition counting only opened
units, threaded through two new positional parameters
(`lossPartialBoardValue`, `lossBvPerSecond`) on
`MainWindow::showEndDialog`. Telemetry gains anonymous
`partial_bv` + `partial_bv_per_second` tags on `game.lost`.

**Why this and not something else:**
- The cycle-23 (v1.26.0) log explicitly parked this as the
  highest-impact next big feature: "Now that the static board 3BV
  (cycle 22) is shipped, the natural next step is the *partial* —
  `Partial 3BV: X / Y · 3BV/s: Z` so a speedrunner sees their
  per-second pace at the moment of explosion."
- The v1.25.0 line told the player how hard the board *was* but said
  nothing about how far *they* got. The partial form is what
  Minesweeper-Arbiter / Minesweeper Online have surfaced for years —
  it's the canonical speedrun progress metric, recognised by every
  player who's looked up "what is 3BV/s".
- The other parked candidates (lifetime "Best %" hall-of-fame for
  never-won Expert, "🎯 Close call!" flair) are smaller-impact
  cosmetic options that can fill future cycles. Partial 3BV is the
  natural progression of the cycle 22 → 23 thread.

**Why replace the cycle-22 line, not append:**
- The new line's Y subsumes the cycle-22 line's value (Y is just
  `boardValue()`). Keeping both would render `Board 3BV: 87 \n
  Partial 3BV: 23 / 87 · 3BV/s: 1.84` — the first line becomes
  redundant noise.
- Cost: 9 hand translations of `Board 3BV: %1` are obsoleted and
  replaced. Benefit: the loss dialog stays at 6 lines (or 7 with
  question marks), no growth past v1.26.0's footprint.
- The cost is acceptable because all 9 translations were freshly
  written 2 cycles ago — no long-tenured strings are being thrown
  away. The replacement strings model on the existing
  `3BV: %1 · 3BV/s: %2` win-dialog conventions for the per-locale
  `/s` suffix (`sn` for TR, `с` for RU, `秒` for ZH, `ث` for AR,
  plain `s` elsewhere), so translators have a consistent house style
  to follow.

**Why a fresh walk, not incremental tracking:**
- A counter would need bookkeeping in `onCellOpened` *plus* knowledge
  of which cells belong to which region — i.e., it would have to
  pre-compute the `compute3BV()` partition at mine-placement time
  and store it as `m_cellRegion[r][c]`. That's persistent state that
  has to be reset by `newGame` / `newGameReplay` / `setFixedLayout`
  (three places) and tested for invariants.
- The walk is `O(rows × cols) ≤ 480` (Expert), single isOpened()
  enum compare per cell, runs *exactly once per game* on the loss
  path (or on win, but only as a postcondition assertion in tests).
  Cost: zero. Cost of incremental tracking: a 2D `int` table sized
  with the grid + reset paths + tests + new state field + risk that
  a future refactor of `onCellOpened` desyncs the counter.
- This is the same lazy-walk pattern used by `questionMarksPlaced()`
  in cycle 23 and would justify `safePercentCleared` being computed
  on read rather than tracked — all are one-shot end-of-game reads
  with no hot-path consumer.

**Why "Cleared 3BV" (Minesweeper-Arbiter) semantics, not per-cell:**
- A region (zero-flood opening) counts as +1 iff *any* cell in it
  (zero or fringe-numbered) is opened, regardless of how many cells
  are opened within it. Matches the canonical speedrun definition
  used by every external tool the player might compare against.
- The alternative — counting per-cell — would diverge from the
  source-of-truth definition and confuse anyone who knows what 3BV
  is. It would also undermine the postcondition
  `partialBV == boardValue()` at the win endpoint, which anchors
  the test suite.

**Why 3BV/s = partial / elapsed, not total / elapsed:**
- A partial-clear rate against the whole board's 3BV would imply the
  player cleared the full board at that pace, which is false — they
  only cleared X of Y. Partial / elapsed is the player's *actual*
  instantaneous pace at the moment of death.
- If the player's pace was held to the end, partial/elapsed = total
  Y / projected total time — it linearly extrapolates to a "would
  finish in" estimate, which is exactly what speedrunners compare
  against their PBs.

**Why a sub-tick guard mirroring the win path:**
- A loss with `m_lastElapsedSeconds <= 0.05` would otherwise divide
  by zero. The win path's `bvRate` already uses this exact guard
  (`(m_lastElapsedSeconds > 0.05) ? (bv / m_lastElapsedSeconds) :
  0.0`); the loss path adopts it verbatim so the line shows `0.00`
  for a sub-tick boom rather than `inf` or NaN.
- Sub-tick losses are reachable in tests (setFixedLayout + immediate
  click on a mine) and theoretically reachable in real play (an
  instant first-click loss before mine-safe placement runs — though
  first-click safety prevents this in normal play).

**Why two new positional parameters on showEndDialog, not packaging:**
- Direct parallel of the cycle-22 reasoning that justified
  `lossBoardValue` and the cycle-23 reasoning that justified
  `lossQuestionMarks` as their own positional parameters. Reusing
  any existing parameter would couple unrelated metric semantics.
- Trailing-only addition keeps existing call-site argument order
  stable. The win-path call site adds `0, 0.0` at the end (the
  partial-bv values are unused on the win path because the dialog
  doesn't render them on wins).

**Why no partial-efficiency line:**
- Efficiency = bv / clicks · 100 needs the *full* bv to be
  meaningful (it's a measure of how optimally the player would
  clear the *whole* board with their click discipline). On a
  partial clear, partial_bv / clicks could be misleading because
  the click count includes any unsatisfied/wrong chord clicks the
  player made trying to recover from confusion. The metric isn't
  comparable to anything the player would recognise. Skip.

**Why no win-dialog parity:**
- The win dialog already shows `3BV: %1 · 3BV/s: %2` (cycle 14). At
  the win endpoint partialBV == boardValue() (anchored by
  `testPartialBoardValueEqualsBoardValueOnWin`), so a partial form
  on wins would be semantically identical and visually noisy.

**Adversarial test verification:** 8 new tests in `tst_minefield.cpp`.
Verified by zeroing the `partialBoardValue()` body and rebuilding —
6 of 8 tests fail (2 baseline tests pass trivially because they
expect 0; that's how to identify which tests are load-bearing for
the value path, not a flaw).

## 2026-04-25 — Loss dialog: question-marks line (v1.26.0)

**Chosen:** Append a seventh line to the loss dialog,
`tr("Question marks: %1").arg(lossQuestionMarks)`, gated
`lossQuestionMarks > 0`. Implement via a new
`MineField::questionMarksPlaced()` accessor that walks the live
`m_buttons` grid counting `isQuestion()`, threaded through a new
positional `lossQuestionMarks` parameter on
`MainWindow::showEndDialog`. Telemetry gains an anonymous integer
`qmarks` tag on `game.lost`.

**Why this and not something else:**
- The cycle-22 log explicitly parked this as the top next candidate
  ("one translatable string, no new state") — the smallest-budget
  high-confidence win on the candidate list.
- It completes the player-action recap. Right-click cycles `None →
  Flag → Question → None`; v1.24 surfaced the Flag step (`Flags
  placed`); leaving the Question step unrepresented makes the recap
  structurally incomplete.
- The other parked candidates (partial 3BV, lifetime "Best %") are
  both substantially larger (~50–80 LOC of new logic) and would
  push the cycle past the 400-LOC cap.

**Why a lazy walk and not a tracked counter:**
- Question is the third step of the right-click cycle; only
  `Flag-on` / `Flag-off` transitions emit `flagToggled`. A counter
  would need either a new `questionToggled` signal subscribed by
  `MineField`, or post-hoc bookkeeping inside `cycleMarker`'s
  Flag→Question and Question→None branches plus a parallel slot.
- The accessor is read *exactly once per game* on the loss path. The
  walk is `O(rows × cols) ≤ 480` (Expert), single enum compare per
  cell. Cost: zero. Cost of the alternative: +1 signal + 1 slot +
  new state field + reset path in three places (newGame,
  newGameReplay, setFixedLayout) + tests for the counter
  invariants.
- Lazy walks for end-of-game-only metrics is the same pattern that
  would justify `safePercentCleared` being computed on read rather
  than tracked — both are one-shot read paths with no hot-path
  consumer.

**Why count cells that turned out to be mines:**
- `revealAllMines` paints mined cells with the explosion icon and
  flips `m_isClicked = true`, but does NOT clear `m_marker`.
  `isQuestion()` stays true after the reveal.
- Semantically, the count is "how many cells the player marked with
  `?`", not "how many `?` survived to the end unrevealed". A `?` on
  a mine still represents the player's act of marking — they marked
  it suspicious, then guessed wrong about which one to step on.
- Pinned by `testQuestionMarksPlacedPreservedOnLoss`, the
  load-bearing test for the new dialog line.

**Why a separate positional parameter, not reusing `flagsPlaced`:**
- Reusing would couple the new gate's semantics to an unrelated
  metric. A future change to `flagsPlaced`'s gating (e.g. suppress
  on replays) would unintentionally flip question marks on/off.
- Parallel positional parameters are the same pattern cycle 22
  established for `lossBoardValue` (own param, not piggy-backed on
  `boardValue`) for the same reason.

**Why `> 0` gating:**
- A common no-`?` loss (especially fast booms) shouldn't render a
  noisy `Question marks: 0`.
- If the user has the question-marks toggle off entirely
  (`MineButton::questionMarksEnabled = false`), `cycleMarker` skips
  Question and the count is structurally always 0 — the gate elides
  the line automatically with no extra branching for that case.

**Skipped:**
- *Question-mark count on the win dialog.* Win-path's `flagAllMines`
  doesn't touch question marks, so the count would be the user's
  true `?` count at victory. But on a win, every mine is
  auto-flagged and any leftover `?` is irrelevant — they were a
  thinking aid for play, not a celebration. Surfacing them on wins
  would reward indecision.
- *Combined `Flags + Question marks: %1 / %2` line.* Saves one `\n`
  but invalidates nine existing hand translations of `Flags placed:
  %1`. Net negative.

**Risk:** None new. Additive — no QSettings/save-format change, no
behavioural change on the win or paused paths, no public API
surface removed.

## 2026-04-25 — Loss dialog: board-3BV line (v1.25.0)

**Chosen:** Append a sixth line to the loss dialog,
`tr("Board 3BV: %1").arg(boardValue)`, gated by `lossBoardValue > 0`.
Surfaces the *board's* canonical difficulty (minimum left-clicks to
clear, no flags or chords) alongside the existing player-action
metrics. `MainWindow::onGameLost` reads `MineField::boardValue()` (an
already-public accessor used by the win dialog), threads it through
`showEndDialog` (new positional int parameter), and includes it as a
`bv` tag on `game.lost` telemetry.

**Why ship this now:**
- The cycle-21 next-candidate list led with this exact line: it
  surfaces the board's *objective difficulty* as a fifth datum,
  complementing the four player-action lines already shown
  (duration, percent-cleared, clicks, flags placed). A player who
  exploded on Beginner with BV=12 sees that the board itself was
  small; a player on Expert with BV=180 sees they were up against an
  unusually open layout. Without the line, the player has no read on
  *what board* they were on — only their own actions.
- Cycle-shaped: production diff is ~10 lines (`mainwindow.{h,cpp}`),
  one new translatable string × 9 hand-translated locales, plus one
  new regression test. No new state, no new MineField surface — the
  data already exists for the win dialog.

**Why on the loss dialog (and *only* the loss dialog, again):**
- The win dialog already shows `3BV: %1 · 3BV/s: %2`. The loss
  dialog showing only the static `Board 3BV: %1` is intentional: a
  partial-clear `bv / m_lastElapsedSeconds` would imply the user
  cleared the *whole* board's 3BV at that pace, which is false. The
  static value is exact; the rate would be misleading.
- A `Partial 3BV` (3BV of the revealed area only) would let us also
  surface a defensible per-second rate on losses, but requires a
  region-walk that mirrors `compute3BV()` limited to opened cells.
  Cycle-20 already costed it at ~50 LOC — over budget for one cycle
  and orthogonal to surfacing the static board value. Park.

**Why `lossBoardValue > 0` gating:**
Mirrors the win-path `boardValue > 0` guard at the existing call site.
`m_boardValue` is initialised to 0 and set to `compute3BV()` either
during `placeMines()` (real play, on first click — guaranteed before
any explosion) or during `setFixedLayout()` (test code path). A loss
with `boardValue == 0` is unreachable in real play and is asserted by
the new `testBoardValuePreservedOnLoss` regression test.

**Why a separate `tr("Board 3BV: %1")` key, not reusing
`tr("3BV: %1 · 3BV/s: %2")` with a stripped suffix:**
- The existing key has the rate suffix baked in across all 9 locales.
  Splitting it would invalidate nine hand translations for a
  cosmetic dedup. Cheaper to ship a fresh key.
- "Board 3BV" (vs. plain "3BV") explicitly anchors the value to the
  board, distinguishing it from the win-dialog rate-paired form for
  any player who reads both dialogs across runs. Translators get a
  parallel shape (e.g. TR "Tahta 3BV: %1", DE "Spielfeld-3BV: %1",
  ZH "棋盘 3BV：%1").

**Why a new positional `lossBoardValue` parameter on `showEndDialog`,
not reusing the existing `boardValue` parameter:**
The existing `boardValue` parameter feeds the win-only "3BV: %1 ·
3BV/s: %2" line. Reusing it would couple the loss-side gate to the
win-side intent — and a future win-side change to suppress the rate
line on certain runs (say, replays) would unintentionally flip the
loss-side line off. Parallel positional parameters keep the win and
loss paths independently controllable. Naming it `lossBoardValue`
documents at the call site that this is the loss path's view of BV.

**Rejected alternatives:**
- *Add a partial-3BV line `Partial 3BV: X / Y` (revealed-area BV
  over total BV).* Most informative loss line conceivable, but needs
  a new region-walking accessor (cycle-20 estimate ~50 LOC) and a
  second translatable string. Park; revisit when the marginal value
  justifies the cost.
- *Move the `3BV: %1 · 3BV/s: %2` win-line key to a "Board 3BV: %1"
  + "3BV/s: %2" pair so loss and win share a key.* The same string
  refactor invalidates 9 hand translations and gains nothing the
  user sees. Skip.
- *Show 3BV on the win dialog as `Board 3BV: X · 3BV/s: Y` for
  symmetry with the loss line.* The win dialog already says `3BV: X
  · 3BV/s: Y` — reflowing it for symmetry would re-translate every
  locale for zero player benefit. Leave the win shape alone.
- *Add a new "🏅 Hard board!" flair when `boardValue` is in some
  high percentile of the difficulty's expected BV.* Needs a
  per-difficulty BV distribution (multi-cycle data collection), and
  the user gets a confidence claim Sentry can't audit. Skip.

**Translation cost:** 1 new string × 9 non-English locales,
hand-written; 0 unfinished per non-en locale preserved. New string:
"Board 3BV: %1".

**Diff shape:**
- `mainwindow.h` — `showEndDialog` signature gains an `int
  lossBoardValue` parameter.
- `mainwindow.cpp` — `onGameLost` reads `boardValue()`, adds it to
  the telemetry tags, threads it; `showEndDialog` appends the gated
  line on the loss branch with the same comment shape as the
  flags-placed and clicks lines from prior cycles.
- `tests/tst_minefield.cpp` — new
  `testBoardValuePreservedOnLoss`: places two fixed mines on a 3×3,
  captures `boardValue()` pre-loss, triggers a loss, asserts
  `boardValue() == initialBV` *and* `initialBV >= 1`. The latter
  pins the > 0 gate's prerequisite (without it, a future zeroing of
  `m_boardValue` on loss would silently swallow the line). Verified
  adversarially by replacing `return m_boardValue` with `return 0`
  and confirming the test fails on the `initialBV >= 1` assert.
- `translations/apply_translations.py` — one new key per locale
  dict, hand-translated.
- `CMakeLists.txt` — version bumped to 1.25.0.

## 2026-04-25 — Loss dialog: flags-placed line (v1.24.0)

**Chosen:** Append a fifth line to the loss dialog,
`tr("Flags placed: %1").arg(flagsPlaced)`, gated by `flagsPlaced > 0`.
Surfaces the user's flag count at the moment of explosion. Adds a
`MineField::flagsPlaced()` getter exposing the existing `m_flagCount`
counter. `MainWindow::onGameLost` reads it, threads it through
`showEndDialog` (new int parameter), and includes it in the `game.lost`
telemetry tags.

**Why ship this now:**
- The last three cycles each added one parallel line to the loss
  dialog — duration (v1.21), percent-cleared (v1.22), clicks (v1.23).
  Flags-placed completes the picture of *user actions* before death:
  alongside Clicks (left-click gestures), the player now sees their
  flag count (right-click gestures). The pair maps to both inputs the
  player has at their disposal.
- Cycle-shaped: ~6-line production diff, 1 new getter, 1 telemetry tag,
  1 new translatable string × 9 hand-translated locales. Same shape as
  the last four cycles.
- Backwards compatible — additive `showEndDialog` parameter; no
  QSettings or save-format change; no behavioural change on the win
  path or any non-end-of-game flow.

**Why on the loss dialog only, not the win dialog:**
The win path's `flagAllMines()` celebratory pass auto-flags every
remaining mine after the state flips to Won. A `flagsPlaced` reading at
that point reflects auto-placed flags, not user intent — surfacing it
would mislead. The loss path's `revealAllMines()` does NOT auto-flag,
so on loss the count is exactly what the user placed before exploding.
Documented in the new `flagsPlaced()` doc comment and pinned by the
`testFlagsPlacedPreservedOnLoss` regression test.

**Why `flagsPlaced > 0` gating:**
Mirrors the v1.23 `userClicks > 0` and the win-path `boardValue > 0`
guards. The common loss path of "first click is a mine" produces zero
flags placed; rendering `Flags placed: 0` is noise. The first explicit
flag a user places lights up the line.

**Why a fresh `MineField::flagsPlaced()` getter (not reading
`remainingMines()` and inverting):**
`remainingMines() = mineCount - flagCount` — it's a derived view, and
inverting it from the dialog code would couple presentation to a
formula that lives in MineField. A direct getter for the underlying
counter is one line and more honest. No behaviour change to
`remainingMines()`.

**Rejected alternatives:**
- *"Flags: %1 (%2 correct)" — also report mines correctly flagged.*
  Requires walking the grid at loss time to count flags-on-mines vs.
  flags-on-safe-cells. Doable but doubles the loss-path work and adds
  another translatable string for a metric that the visible board
  already shows (wrong flags get rendered with the red-X overlay by
  `revealAsWrongFlag()`). Park.
- *"Flags placed: X / Y" with Y = total mines.* Spoils the mine count
  at the end of the run; some players use the LCD as their primary
  reference and may not have memorised the difficulty's mine count.
  Kept it as a single number to mirror the existing `Clicks: %1`
  shape. Park.
- *Adding the line on the win dialog too.* The auto-flag inflation
  (see above) makes the count uninteresting on a win — every mine is
  flagged by definition. Skip.
- *Showing flag-rate (flags/sec) like 3BV/s.* Not a recognised
  Minesweeper community metric, and lossy on short runs. Skip.

**Translation cost:** 1 new string × 9 non-English locales,
hand-written; 50/50 finished/unfinished coverage preserved. New
string: "Flags placed: %1".

**Diff shape:**
- `minefield.h` — `flagsPlaced()` declaration with doc.
- `minefield.cpp` — one-line getter.
- `mainwindow.h` — extra `int flagsPlaced` parameter on
  `showEndDialog`.
- `mainwindow.cpp` — read `flagsPlaced()`, thread to dialog, render
  line on loss path, telemetry tag.
- `tests/tst_minefield.cpp` — five new regression tests
  (`testFlagsPlaced{ZeroBeforeAnyFlag,IncrementsOnFlag,
  DecrementsOnUnflag,PreservedOnLoss,ResetByNewGame}`).
- `translations/apply_translations.py` + nine `.ts` files via
  `lupdate` + `python3 apply_translations.py`.

## 2026-04-25 — Loss dialog: clicks line (v1.23.0)

**Chosen:** Append a fourth line to the loss dialog,
`tr("Clicks: %1").arg(userClicks)`, gated by `userClicks > 0`.
Surfaces the run's useful-click count at the moment of explosion, mirroring
the second half of the win dialog's `Clicks: %1 · Efficiency: %2%` line
(minus the efficiency suffix — see below). `MainWindow::onGameLost` now
captures `userClicks` from `MineField`, threads it through `showEndDialog`,
and includes it in the `game.lost` telemetry tags for parity with `game.won`.

**Why ship this now:**
- v1.22.0 added `You cleared %1% of the board.` to the loss dialog —
  surfacing **progress**. This cycle adds **effort**: clicks. A user who
  reached 60% in 25 clicks vs. 60% in 80 clicks now has the data to
  recognise the difference and adjust play.
- The next-candidates list in cycle 19 explicitly named this pattern
  (`4th line surfacing the run's 3BV efficiency at the moment of
  explosion, mirrors the win dialog's 3BV/s line`). The implementation
  diverges from that wording on purpose — see "Why clicks, not 3BV/s"
  below.
- Cycle-shaped: ~5-line production diff, 1 telemetry tag, 1 new
  translatable string × 9 hand-translated locales. Same shape as the
  last four cycles.
- Backwards compatible — additive `showEndDialog` parameter wiring only;
  no QSettings or save-format change; no behavioural change on the win
  path.

**Why clicks, not 3BV/s:**
The win-dialog 3BV/s line computes `boardValue / m_lastElapsedSeconds` —
i.e. *total* board complexity divided by *total* run time. On a loss,
`boardValue` still reads the **whole board's** 3BV (computed once at
mine-placement and cached), but the user only **completed a fraction**
of it. Reporting `boardValue / loss_seconds` would imply the user
cleared the entire 3BV at that rate, which is false. Computing partial
3BV (the 3BV of the currently-revealed area) requires new code that
walks the revealed openings and numbered cells; out of scope for a
one-line dialog patch. Click count is exact, requires no new
computation, and is the half of the win-dialog metric pair that survives
unmodified to a loss. Users can divide the percent-cleared line by the
clicks line themselves to gauge their own efficiency relative to past
runs.

**Why a separate `tr("Clicks: %1")` key vs. reusing the win-dialog one:**
The win-dialog line is `tr("Clicks: %1 · Efficiency: %2%")` — a single
translatable unit so locales control the separator and order. Splitting
that into two atoms for partial reuse on the loss path would invalidate
nine existing translations and give translators a less coherent unit to
work with. A dedicated `tr("Clicks: %1")` is one short string; cheaper
than the alternative.

**Why `userClicks > 0` gating:**
Mirrors the win-dialog `if (userClicks > 0)` guard. The only paths that
trigger a loss with zero useful clicks are pathological test setups
(`setFixedLayout` + an explosion fired before any reveal); on those
paths a `Clicks: 0` line is noise.

**Rejected alternatives:**

- **Mirror the entire win-dialog block on loss** (3BV, 3BV/s, Clicks,
  Efficiency). Misleading — see "Why clicks, not 3BV/s" above. Reporting
  efficiency = 3BV / clicks · 100 on a partial board is also wrong (3BV
  here is the *whole* board, not the part the user reached).
- **Define a loss-specific "partial efficiency" metric.** E.g.
  `safe_revealed / clicks · 100`. Plausible but a new metric the user
  has to learn vs. the win-side definition. One cycle, one line — keep
  it crisp.
- **Compute partial 3BV (3BV of the revealed region) and surface
  `Partial 3BV: X / Y · 3BV/s: Z`.** Requires walking the revealed
  openings and numbered cells with the same connectivity rules as
  `compute3BV()` but limited to opened cells. Real implementation cost
  beyond a one-line dialog patch; deferred.
- **Prose form `tr("You used %1 clicks.")`** to match the existing
  prose lines on the loss dialog (`You stepped on…`, `You survived…`,
  `You cleared…`). Considered. Rejected because (a) singular/plural
  handling at `1 click` is awkward for nine languages without `%n`
  plural forms, and (b) the terse `Clicks: %1` format is already
  established in the win dialog and matches translators' existing key.
- **Skip the gate, always show.** A loss with `Clicks: 0` is a test
  artefact (chord-on-fixed-layout explosion before any reveal). Showing
  it on the user-visible path adds zero info; cheap to gate.

**Assumptions:**
- Users want to know how many gestures they made. Click count is a
  metric speedrunners and casual players both understand.
- Telemetry parity matters — the `game.lost` event already records
  `duration_seconds` and `replay`; adding `clicks` makes the lost-event
  schema match the won-event schema for the click metric.
- The `tr("Clicks: %1")` source string is short enough that all nine
  locales can render it in one printable token + the number, without
  layout-busting expansion in any RTL or Asian locale.

**Translation surface:** 1 new string × 9 hand-translated locales (TR,
ZH, HI, ES, AR, FR, RU, PT, DE). All ten `.ts` files complete
(`<translation>` populated, no `unfinished`) before commit.

## 2026-04-25 — Loss dialog: percent-cleared line (v1.22.0)

**Chosen:** Add a third line to the loss dialog only,
`tr("You cleared %1% of the board.").arg(safePercentCleared())`,
exposing a new `MineField::safePercentCleared()` integer accessor that
reports the percentage of safe cells revealed at the moment of
explosion. Round-half-up via integer arithmetic.

**Rejected alternatives:**
- *Cell-count form ("87 of 99 cells")* — same information, more text,
  noisier. Percent is the canonical "almost-won" metric.
- *Show on the win dialog too.* A win is provably 100%, so the line
  carries zero new information; would just dilute the existing one-
  line "You won!" celebration.
- *Floating-point percentage ("87.4%")* — sub-percent precision is
  meaningless for a game stat and adds locale-specific decimal
  separator handling for nine translations.
- *Round-half-to-even (banker's)* — statistically tidier but not what
  a player expects from a single-digit UI metric. A 99.5% near-win
  rendering as "99%" would feel cruel.

**Why ship this now:**
- Builds directly on v1.21.0's `You survived for %1.` line — the loss
  dialog is now in active iteration and the marginal cost of one more
  line is low.
- Concrete user value — converts a near-miss from an abstract
  `boom` into a quantified `you were 87% there`.
- Zero new bookkeeping. `m_openedSafeCount` is already maintained for
  the win-detection path; we just expose it in the right form.
- Backwards compatible — additive `MineField` API, no save-state
  change, no win-path change.

**Why round-half-up via integer arithmetic:**
`(opened * 100 + total/2) / total`. Avoids float, avoids locale
decimal-separator issues, avoids the 99.5%-renders-as-99% cruelty.
Tested at the 1/3 (33%) and 2/3 (67%) boundary.

**Translation surface:** 1 new string × 9 hand-translated locales.
Hindi and Arabic entries were missing from the in-progress
`apply_translations.py` when the feature work landed in the tree — added
before commit. All 10 `.ts` files now have a complete
`<translation>...</translation>` for the new key.

**Assumptions:**
- Single-digit precision is the right granularity for a one-line UI
  stat. Players will not tell the difference between 87% and 87.4%.
- Loss-only placement matches the spirit of the line ("you almost
  made it"). Wins don't need a "you cleared 100%" confirmation.

## 2026-04-25 — Win-dialog adopts MM:SS clock format (v1.20.0)

**Chosen:** Replace the win-dialog's
`tr("You cleared the field in %1 seconds.").arg(QString::asprintf("%.1f", m_lastElapsedSeconds))`
with `tr("You cleared the field in %1.").arg(formatElapsedTime(m_lastElapsedSeconds))`.
The cleared-time line now renders the duration in the same
`S.S` / `M:SS.S` / `H:MM:SS.S` shape as the live toolbar timer
(v1.18.0) and the stats-dialog *Best time* / *Best (no flag)*
columns (v1.19.0). The literal `seconds` noun is dropped from
the source string; the format itself carries the unit, identical
to the other two surfaces.

**Why this one (cycle 17):** Cycle 15 (v1.18.0) and Cycle 16
(v1.19.0) both explicitly listed this as the natural follow-up
and parked it on translation cost. With both earlier cycles in
production for two cycles each and zero Sentry hits across both
post-release watches, the format contract is proven and the
win-dialog is the only remaining surface that still reads in raw
decimal seconds. Closing this loop now means every elapsed-time
surface in the app uses one shared formatter.

**Rejected alternatives:**

- **Substitute "seconds" with "time" or "minutes" instead of
  dropping it.** "You cleared the field in 1:30.5 minutes." is
  factually wrong (1:30.5 is one minute thirty-and-a-half
  seconds, not 90.5 minutes); "in 1:30.5 time." is awkward in
  English and more so in target languages. Dropping the noun
  entirely matches both prior surfaces and reads cleanly.
- **Keep "seconds" only when the format is bare-decimal.**
  `"You cleared the field in 45.0 seconds."` for sub-60 runs +
  `"You cleared the field in 1:30.5."` for longer ones. Mixed
  format inside the same dialog across runs is exactly the
  parity-gap class of problem this cycle came to close.
- **Lift the dialog text into a free function in
  `time_format.h` for direct testing.** Adds `tr()` wiring and
  a translation context to a header that today is one
  branchless formatter. Single call site; the format half is
  already pinned by 14 cases in `tst_time_format`.
- **Add a `tst_mainwindow` test that constructs a `MainWindow`,
  drives a fake game to a win, and snapshots the dialog
  string.** Way over-budget for a one-line call swap; same
  scaffolding cost the earlier cycles rejected.
- **Defer one more cycle to ship `Hint button` first.** The
  hint-button slice is at least 250-400 LOC and needs a
  deterministic solver; the win-dialog parity is concrete and
  cheap. Cycle slots are 1/cycle — pick the cleaner ship.

**Assumptions:**

- The format itself carries the unit — both prior surfaces
  proved players read `1:30.5` as a duration without a unit
  word, and the win dialog header (`You won!`) already frames
  the context.
- Stored `bestSeconds` / `bestNoflagSeconds` in QSettings
  remain byte-identical; only the dialog rendering path
  changes. Legacy records render in the new format on the
  next win with no migration step.
- The defensive `seconds <= 0.0` / NaN / inf paths in
  `formatElapsedTime` already return `"0.0"`, so a degenerate
  win clock can't reach the user as `nan` or a negative number.

**Translation strategy:**

- All 9 hand-locales drop the unit noun in place. No verb
  refactor required for 8 of 9.
- Turkish needs a temporal particle in place of `saniyede`
  (`saniyede` = "in seconds" with the locative on the noun).
  Direct drop reads ungrammatical. Use
  `"Alanı %1 içinde temizlediniz."` ("You cleared the field
  within %1.") — the locative attaches to the new postposition
  `içinde` and the string reads naturally with a colon-clock
  placeholder.
- `lupdate` confirms 92 finished / 0 unfinished across all 10
  locales. The previous translation drops to `type="vanished"`
  per `lupdate`'s deprecated-entry bookkeeping (not
  user-visible; just keeps the old translation around in case
  the source string ever reverts).

**Risks considered, none load-bearing:**

- *Mixed-format ambiguity.* "1:30.5" alone could be parsed by
  some users as "one and a half hours", but the live timer +
  stats columns have shipped this exact format for two cycles
  with zero feedback or telemetry events flagging it.
- *Translation regression in a locale I don't read fluently.*
  Mitigated by keeping the substitution mechanical (drop the
  unit-noun) and validating that `lupdate` shows finished /
  not-unfinished; semantic correctness verified on tr / en /
  de / es / fr / pt by reading; ru / zh / hi / ar verified
  structurally (mechanical noun-drop, no verb refactor).
- *PR diff bloat from `.ts` regeneration.* +760/-724 lines in
  the diff is mostly `lupdate` rewriting line-number metadata
  in 10 `.ts` files. Reviewer-friendliness is fine because the
  *semantic* diff is 12 lines: 2 in `mainwindow.cpp`, 1 in
  `CMakeLists.txt`, 9 in `apply_translations.py`.

## 2026-04-25 — Stats-dialog adopts MM:SS clock format (v1.19.0)

**Chosen:** Replace the stats-dialog `formatBest` lambda's
`QString::asprintf("%.1f s", seconds)` with a single call to the
duration-aware `formatElapsedTime(seconds)` shipped in v1.18.0.
The "Best time" / "Best (no flag)" columns now render as
`S.S`, `M:SS.S`, or `H:MM:SS.S` depending on duration, matching
the live toolbar clock. The trailing `" s"` unit is dropped; the
column header carries the unit and `s` reads wrong on a
colon-clock value.

**Why this one (cycle 16):** v1.18.0's CYCLES entry explicitly
listed this as the "natural follow-up" — its scope (the live
label) was contained, and the stats column was deliberately
deferred so the live format could age. With v1.18.0 in the wild
and zero Sentry hits in the post-release watch, the contract is
proven and the parity gap is the next cleanest visible polish.
Five-minute cycle, ~6 LOC of productive change, zero new
translatable strings, zero persistence churn, formatter already
covered by 14 unit tests.

**Rejected alternatives:**

- **Keep the unit conditionally** (`"45.0 s"` for sub-60,
  `"1:30.5"` past it). Mixed format inside one column is
  visually inconsistent and contradicts the goal of matching
  the live clock.
- **Lift `formatBest` to a free function in `time_format.h`**
  so it could be unit-tested directly. Adds `QLocale` and
  `QDate` to a header that today has only `QString`. Single
  call site, no real testability win — the format part is
  already pinned by `tst_time_format.cpp`, and the date-pairing
  half is locale-formatted boilerplate.
- **Also reformat the win-dialog "You cleared the field in %1
  seconds." message.** Would touch a translated string and burn
  9 hand-translations on a cosmetic change. v1.18.0 already
  rejected the same scope expansion for the same reason; keep
  the cycle additive.
- **Add a duration suffix only when the format wraps** (e.g.
  `"45.0 s"` < 60, `"1:30.5"` past). Same mixed-format problem.
- **Ship a stats-dialog refactor that lifts everything into a
  `Stats::DialogModel` class** so the rendering helpers are
  testable. Premature; the existing showStatsDialog is small
  and the new line is one call swap.

**Assumptions:**

- The "Best time" / "Best (no flag)" column headers carry the
  unit clearly enough that dropping the literal `s` doesn't
  introduce ambiguity. Other Minesweeper-family games (KMines,
  GNOME Mines) ship without per-cell units in these columns.
- Stats values stored as `double` seconds in QSettings remain
  byte-identical; only their string rendering changes. Legacy
  records render in the new format on first dialog open with
  no migration step.
- `formatElapsedTime` already handles the defensive cases
  (`<= 0`, `NaN`, infinity) — but the lambda's own
  `seconds <= 0.0` early-return for "no record yet" stays in
  place because the rendering needs to emit `"—"`, not `"0.0"`.

## 2026-04-25 — MM:SS timer format for long runs (v1.18.0)

**Chosen:** Switch the live timer label from a fixed-width
`%05.1f` (e.g. `0123.4`) to a duration-aware formatter. Under 60
seconds the display reads `S.S` / `SS.S`; from 60 s up to one hour
it reads `M:SS.S` / `MM:SS.S`; past one hour it reads `H:MM:SS.S`.
The `MineField` widget itself is untouched — only the toolbar
clock changes. Stored seconds (`bestSeconds`, `bestNoflagSeconds`,
telemetry `duration_seconds`) and the win-dialog "you cleared the
field in N seconds" string keep their decimal-seconds form, so
nothing about the persistence schema or translation contract
shifts. The formatter is a pure inline function in a new
`time_format.h` header so unit tests link it without touching the
core library.

**Why this one (cycle 15):** The `%05.1f` format truncates after
`999.9` — Expert games that run more than ~17 minutes either
render as a wider-than-expected `1234.5` (drifts the toolbar) or
get read by the player as a four-digit number rather than a
duration. The fix has been parked twice in the Next-candidates
list as "small polish, zero new translatable strings"; v1.17.0
cleared the queue's bigger design item (color-blind palette) and
this is the smallest shippable thing left. Strict additive change
to `updateTimerLabel()`, no schema migration, no stats/telemetry
re-emission, ~10 LOC of formatter logic and ~80 LOC of tests.

**Rejected alternatives:**

- **MM:SS without sub-second precision.** Friendlier to read but
  drops the speedrun-relevant tenth-of-a-second granularity. The
  whole speedrun trio (3BV, 3BV/s, efficiency) shipped in
  v1.14–v1.15 already; surrendering tenths for the live counter
  would un-do that direction.
- **Hard-cap the timer at 999.9 like classic Windows
  Minesweeper.** Hides truth from the player. With a working
  pause and per-difficulty stats, "how long did you take" is a
  legitimate signal — the format just needs to scale.
- **Always render `MM:SS.S` (with leading `0:` for runs under 60
  seconds).** Reads as awkward (`0:05.3`) for the typical
  Beginner game which finishes in under 30 s. Conditional format
  matches the user's mental model.
- **Localised `1m 30.5s` style.** Adds 1–2 new tr() strings × 9
  locales (≥ 9 hand translations). The colon-separated form is
  universally readable as a clock and doesn't need translation.
- **Also reformat the stats-dialog best-time column.** Tempted —
  it'd be consistent. But the stats column already pairs with a
  unit (`42.7 s`); changing it to `1:30.5` removes the unit and
  changes the column-width budget. Not a 400-LOC-cap cycle's
  worth of risk; defer to a future polish cycle once the live
  format has aged in production.
- **Also reformat the win dialog's seconds string.** Would touch
  the existing translated string `You cleared the field in %1
  seconds.` — every locale would either need a new conditional
  variant or an unfinished entry. Strictly out of scope for a
  zero-translation-churn cycle.

**Assumptions:**

- The live label is right-aligned (`AlignRight`) in a flexible
  layout cell, so the variable-width result (`8.4` vs `1:23.4`)
  doesn't disturb other widgets in the toolbar — the right edge
  stays pinned.
- `formatElapsedTime` clamps negatives and `NaN` to `"0.0"`. The
  call site won't pass either today, but defensive output beats
  surfacing a `nan` to the user if the elapsed math ever drifts.
- The placeholder reset ("Ready" state) shifts from the literal
  `000.0` to `0.0`. Acceptable visual delta — the reset state is
  brief and immediately overwritten on first click.

## 2026-04-25 — Color-blind friendly number palette (v1.17.0)

**Chosen:** Add an opt-in `Settings → Color-blind friendly numbers`
toggle (checkable QAction, default off). When enabled, `MineButton`'s
`numberColor()` returns an Okabe-Ito-derived palette instead of the
classic Minesweeper 1–8 palette. The new palette keeps every digit
distinguishable under deuteranopia and protanopia — the classic
palette's 2/3 (green/red) and 5 (dark red) collide under red-green
CVD. App-wide static state on `MineButton` (mirrors the
question-marks pattern). Persisted as QSettings
`settings/colorblind_palette`. Toggling mid-game sweeps the grid via
a new `MineField::refreshAllNumberStyles()` → `MineButton::refreshNumberStyle()`
helper so the change is visible on already-opened cells.

**Why this one (cycle 14):** Parked for three cycles running as
"design-level work" in the Next-candidates list (v1.13, v1.14,
v1.15, v1.16). The design work isn't actually open-ended — the
Okabe-Ito palette is a published, peer-reviewed 8-colour set
specifically for CVD-safe categorical data. Picking that palette
removes the "need to design" obstacle and leaves a small diff:
~60 LOC core + wiring, 1 new translatable string × 9 locales, 4
new unit tests. Accessibility payoff is real (≈8 % of male players
have red-green CVD). Opt-in default keeps the classic look for
everyone else. Diff is well under the 400-LOC cycle cap.

**Rejected alternatives:**

- **Pattern overlays on digits (dots/stripes).** Considered a
  pure-shape distinguisher that works for total colour blindness.
  Cells are 30×30 px with a ~22 px digit; overlay patterns would
  clash with the numeric glyph and read as visual noise. A palette
  swap keeps the interface clean.
- **Full high-contrast / "dark mode" theme.** Much wider surface
  — every stylesheet (opened cell, mine, wrong-flag, pause
  overlay, smiley button, menu text) would have to be themed.
  Multi-cycle work; the number palette is the single highest-value
  accessibility slice.
- **Per-digit individual colour picker.** Interesting config
  fidelity but the scope explosion (9 new strings for the dialog,
  serialization schema) dwarfs the feature's actual accessibility
  payoff.
- **Auto-detect via OS accessibility settings.** Qt doesn't expose
  a colour-blind hint on macOS/Windows; parsing `QPalette` would
  be a heuristic. An explicit user toggle avoids guessing.
- **Default on.** Would change the look for every existing player
  without warning — the classic palette is what the muscle memory
  is trained on. Opt-in mirrors the GNOME Mines and Microsoft
  Minesweeper model.
- **Silent mid-game toggle.** I.e. change the flag, but only new
  reveals use the new palette (already-opened cells keep the old
  colours until next game). Cheaper to implement (no refresh
  helper), but the UX is confusing — the user flips the toggle,
  nothing visible happens, and they can't tell if it worked.
  `refreshAllNumberStyles()` is a ~20 LOC addition for a much
  clearer feedback loop.
- **Apply palette to the `number=0` opened-cell background.** No
  digit is drawn on a zero cell, so there's nothing to recolour;
  the guard in `refreshNumberStyle()` skips zeros to avoid
  needlessly churning their stylesheets.

## 2026-04-25 — Win streak per difficulty (v1.16.0)

**Chosen:** Track current and best win streak per difficulty in the
existing per-difficulty `Stats::Record`. `recordWin` increments
`currentStreak`, updates `bestStreak` when current exceeds it, and
stamps `bestStreakDate` on the high-water-mark moment. `recordLoss`
zeroes `currentStreak` (per the difficulty being played) and leaves
the best alone. Surface a "Streak" column in the Statistics dialog
(`current/best`, `—` when both are zero) and a `🔥 Streak: N` flair
on the win dialog when current ≥ 2, swapped for `🌟 New best streak!`
when the high-water mark grows. Replays and Custom games stay
excluded — same exclusion rule as `played`/`won`/`bestSeconds`, so
neither a memorised-board win nor a custom-grid loss can game the
streak.

**Why this one (cycle 13):** Last cycle's "Next candidates" list
re-stated the same four parked-multiple-times items (save-and-resume,
per-layout leaderboard, color-blind palette, hint button), every one
of which is multi-cycle work or design-heavy. Win streak slots in
underneath all of them: small (~150 LOC core), zero risk to game
logic, additive Stats schema (legacy records load with zero streak),
and engagement payoff is real — it pairs naturally with the
speedrun-aware direction (best-time-with-date in v1.3.0, no-flag
bracket in v1.13.0, 3BV/s in v1.14.0, efficiency in v1.15.0). One
cycle, one feature, ships clean.

**Rejected alternatives:**

- **Global streak (not per-difficulty).** A Beginner win after an
  Expert loss would extend the streak; conceptually muddled and
  unfair to the player switching difficulties. Per-difficulty
  matches the rest of the schema.
- **Reset other difficulties' streaks on a loss.** A Beginner loss
  doesn't say anything about Expert skill; cross-difficulty resets
  punish exploration. Per-difficulty losses only.
- **Time-window streaks (e.g. last 24h).** Adds a clock-based
  invalidation path and complicates persistence. Lifetime streaks
  match `played`/`won`/`bestSeconds`'s lifetime semantics.
- **Track streak length history (last N) for an "average streak"
  number.** Inflates schema for marginal display value. Two scalars
  (current + best) cover 95 % of the engagement story.
- **Only show on the win dialog (no Stats column).** Hides the best
  streak when the user wants to look it up between games. The
  Statistics dialog is the one place to see lifetime aggregates;
  streak belongs there.
- **Pre-deduct on first-click loss.** First-click loss can't happen
  on a real game (first-click safety) but can on replay. Replays are
  excluded from streaks anyway, so no special-case needed.
- **Reset on `New Game` mid-run.** Conceptually — abandoning a game
  and starting another is a "loss" of intent. But it's never been
  counted as a loss in `played`, so streak follows the same rule:
  only an explicit Lost state resets. Avoids surprising the user
  who hits Ctrl+N out of curiosity.

## 2026-04-25 — Click count + Efficiency % metric (v1.15.0)

**Chosen:** Track every user gesture that reveals at least one cell —
left-click reveal, mouse chord that opens ≥1 neighbour, keyboard
Space/Enter reveal, keyboard Space/Enter/D chord that opens ≥1
neighbour — as a single "useful click" on `MineField::m_userClicks`.
Surface `Clicks` and `Efficiency = 3BV / clicks · 100` on the win
dialog as a second speedrun-footer line under the existing 3BV/s
line. Telemetry tags `clicks` and `efficiency` are added to the
existing `game.won` event. No Stats column, no live ticker, no
schema break.

**Why this one (cycle 12):** Direct follow-on from v1.14.0's 3BV/s
— that cycle explicitly parked Efficiency % "for a follow-on cycle if
a click-count is wanted for other reasons." Click count is a useful
standalone stat alongside efficiency; chord-heavy play can yield
>100 % which surprises users in a good way and is a natural reward
for skill. Display-only addition: zero schema break, zero risk to the
state machine. The plumbing is small — a new `MineButton::userClick`
signal for the mouse-left-click path plus inline increments in
`MineField::handleCellKey` and `onChordRequested`. Diff well under
400 LOC.

**Rejected alternatives:**

- **Count every left-click + chord, useful or not.** Standard
  "total clicks" definition. Easier to compute, but harder to
  interpret — clicks on already-opened cells, on flagged cells, and
  unsatisfied chords would inflate the denominator and depress
  efficiency for no skill-related reason. The "useful click"
  definition matches Minesweeper Online's `3BV / 3BV-clicks` model
  more closely and rewards what players actually optimise for.
- **Cap efficiency at 100 %.** Capping discards information; chord
  gestures legitimately open multiple BV cells in one click and the
  resulting >100 % is the right answer. Speedrun communities report
  uncapped efficiency.
- **Show clicks alongside 3BV on the same line.** Would force the
  existing `"3BV: %1 · 3BV/s: %2"` format string into a new shape and
  unfinish all 9 hand-translations. Adding a separate
  `"Clicks: %1 · Efficiency: %2%"` line keeps the existing string
  untouched and only costs 9 fresh translations.
- **Track clicks as a Stats column / per-difficulty best efficiency.**
  Already 5 columns; another bracket per metric inflates the schema
  for a per-run-shape number. Display-only on the dialog preserves
  the metric without the schema cost — same trade as 3BV/s in v1.14.
- **Increment in `MineButton::Open()` itself.** `Open()` is also
  called from flood-fill (`onCheckNeighbours`) and chord neighbour
  loops; counting there would over-count by the size of every flood,
  not by user gestures. Counting at the gesture entry points
  (mousePressEvent, handleCellKey, onChordRequested) is the only
  correct cut.
- **Count the chord cell itself as part of the gesture.** No — the
  chord cell is already opened (precondition `cell->isOpened()`). The
  "useful click" is the *act* of chording, scored once per gesture
  iff at least one neighbour was actually revealed.
- **Show efficiency on the loss dialog.** A losing board never reaches
  full clear, so "efficiency" is meaningless before the run ended;
  3BV/s is similarly skipped on losses (cycle 11).
- **Live efficiency ticker during play.** Same reason as 3BV/s in
  cycle 11: speedrunners measure final figures, not running ones.

**Assumptions documented:**

- **Useful click = gesture that opened ≥1 cell.** Defines the
  denominator. Right-clicks (flag toggles) never count. A left-click
  on an already-opened or flagged cell is a no-op and never counts.
  An unsatisfied chord (flags ≠ number) returns early before opening
  anything and never counts. A satisfied chord with all neighbours
  already opened or flagged opens nothing and never counts.
- **Chord that hits a wrong-flag mine still counts.** It opened a
  cell (the mine). Even though the run ends in a loss, the gesture
  was a useful click; counting it is consistent with the definition
  and only matters in telemetry — the loss dialog doesn't show
  efficiency.
- **Flood-fill propagation is one click.** A single left-click that
  cascades through a 50-cell zero region is 1 useful click, not 50.
  This is the speedrun-canonical interpretation and what makes BV/s
  and efficiency meaningful.
- **Replay and Custom wins get the metric too.** Same rationale as
  3BV/s in cycle 11: it's a property of the run, not a leaderboard
  claim. Replay-vs-replay efficiency comparisons on the same layout
  are genuinely useful for self-coaching.
- **Suppressed on `clicks == 0`.** Fixed-layout test setups can
  reach `onGameWon` without any user gesture; the dialog skips the
  efficiency line when there are no clicks rather than printing
  "Clicks: 0 · Efficiency: 0%". Mirrors the `bv == 0` guard from
  cycle 11.

## 2026-04-25 — 3BV + 3BV/s efficiency metric (v1.14.0)

**Chosen:** Compute the 3BV (Board Value) — the canonical Minesweeper
speedrun metric for "minimum left-clicks to clear" — once when the
mines are placed, cache it on `MineField`, and surface it together
with the per-second rate (3BV/s) on the win dialog. Telemetry tags
`bv` and `bv_per_second` are added to the existing `game.won` event.
No persistence, no Stats column, no UI rearrangement.

**Why this one (cycle 11):** Natural follow-on from v1.13.0's no-flag
bracket — both are speedrun-community-canonical metrics that the
project's existing rhythm rewards (best-time-with-date in 1.3.0,
no-flag bracket in 1.13.0). Display-only addition: zero schema break,
zero risk to game logic. Diff well under 400 LOC. Builds on the
already-shipped `m_lastElapsedSeconds` and `boardValue()` is a pure
read after mines are placed.

**Rejected alternatives:**

- **Track 3BV/s as a Stats column.** Would multiply the per-difficulty
  bracket count again (now 6: best, no-flag-best, best-3BV/s — and
  each invites a `_date` companion). Display-only on the win dialog
  preserves the metric's value without inflating the schema.
- **Compute 3BV/s incrementally during play (board efficiency live
  ticker).** Distracting and not what speedrun players want — they
  measure final-time-rate, not running-rate. Skip the timer-tick
  bookkeeping.
- **"Efficiency %" = 3BV / clicks.** Would require tracking total
  clicks (a new MineField counter) for marginal payoff over 3BV/s.
  Park.
- **Show 3BV on the lost-dialog too.** A loss reveals the board, so
  3BV is computable, but 3BV/s is meaningless (the player didn't
  clear). Better to keep the metric tied to wins where the rate is
  the headline number.
- **Custom and Replay get 3BV too.** Yes — the metric is a property
  of the *run*, not a leaderboard claim. Replays can compare 3BV/s
  attempts on the same layout (genuinely useful), and Custom games
  can report 3BV (unique to that grid). Excluding them would make
  the dialog inconsistent for the same view-only metric.

**Implementation invariants:**

- **Computed once per layout, cached.** `m_boardValue` is set in
  `onCellPressed` (after `fillMines` + `fillNumbers`), in
  `newGameReplay` (after re-applying mines + `fillNumbers`), and in
  `setFixedLayout` (after `fillNumbers`). Reset to 0 in `newGame`
  (mines aren't placed until the first click) and in `setFixedLayout`'s
  reset block.
- **Pure const, no side-effects.** `compute3BV()` is `const` and reads
  only `MineButton::isMined()` and `Number()`. Safe to call from any
  read-side path. `boardValue()` is a trivial accessor.
- **3BV algorithm.** Two-pass: pass 1 flood-fills each connected zero
  region (BFS through 8-neighborhood) and marks fringe numbered cells
  as visited; +1 BV per region. Pass 2 counts every unvisited non-mine
  cell as +1 (these are isolated numbered cells not adjacent to any
  zero — they require their own click). Mines are never counted.
- **3BV/s div-by-zero guard.** `m_lastElapsedSeconds > 0.05` floor
  before dividing. In real play the timer always advances at least
  0.1s before `onGameWon` fires; the guard exists for the test
  pathway through `setFixedLayout` where elapsed time is zero.
- **No new GameState transitions.** The metric is read post-win in
  `onGameWon`; the state machine is untouched.
- **Dialog format.** Appended as a new newline below the existing
  "You cleared the field in X seconds." line, not interleaved with
  the 🏆 / 🏃 prefixes — those are run-quality indicators, 3BV is
  run-shape data. Suppressed when `boardValue == 0` (defensive: a
  hand-rolled `setFixedLayout` win during a test mid-sweep).
- **Telemetry semantics.** `bv` is integer; `bv_per_second` is float
  with two decimal places. Both tag the existing `game.won` event,
  joining the established `noflag`/`new_record`/`replay` tags.

**Risks / mitigations:**

- **Compute cost on Expert.** 30×16 = 480 cells × 9-neighborhood BFS
  = ~4320 visit ops worst case. Trivial; runs once per game-start in
  microseconds. Verified on macOS dev build via headless ctest under
  the existing 0.73s `tst_minefield` budget (no measurable runtime
  delta).
- **Translation cost.** 1 new format string × 9 non-English locales.
  "3BV/s" is internationally invariant in the speedrun community;
  hand-translated only the "/s" abbreviation where a clear local
  convention exists (RU "/с", ZH "/秒", AR "/ث", TR "/sn", FR
  spacing rules) — others stay "3BV/s" verbatim. All 10 locales
  87/87 finished.
- **Dialog width.** Worst-case Expert run: "3BV: 290 · 3BV/s:
  3.41" — about 22 chars, fits inside the existing
  cleared-the-field-in-X-seconds line. No overflow.
- **`m_boardValue` stale after a difficulty change with mines never
  placed.** Reset to 0 in `newGame()` so the cache cannot leak from
  a previous game. Tested in
  `testBoardValueResetByNewGame`.

## 2026-04-25 — No-flag speedrun bracket (v1.13.0)

**Chosen:** Track per-game whether the player ever placed a flag, and
when a win occurs without one, record a separate per-difficulty "Best
(no flag)" time. Surface it on the end-of-game dialog as a
`🏃 No-flag run!` badge (stacked above the existing `🏆 New record!`
when applicable) and as a 5th column in `Game → Statistics…`. Excluded
from Custom and Replay runs (consistent with the existing best-time
exclusion).

**Why this one (cycle 11):** Lowest-risk parked candidate from cycle
10. Pure additive — no rule changes, no UI rearrangement, no schema
break. Speedrun depth without a "hardcore mode" toggle: flagging is
still encouraged for normal play, the bracket just rewards players who
opt out implicitly. Diff stays well under 400 LOC.

**Rejected alternatives:**

- **Hardcore mode toggle that disables flagging.** Would force a UX
  decision (menu item? difficulty variant?) and split the player base
  across two leaderboards before there's even one. The implicit-bracket
  approach gets 90% of the value with zero new mode surface.
- **Track flag count not just presence.** Tempting (e.g. "best with ≤5
  flags") but multiplies the bracket count and adds tuning knobs we'd
  have to defend. Binary flag/no-flag is the cleanest cut.
- **Combine into single Best column with a 🏃 badge if no-flag.** Would
  hide the parallel record. Players who flag normally would never see
  a no-flag time, removing the aspirational pull. Two columns it is.

**Implementation invariants:**

- **Sticky bit.** `MineField::m_anyFlagPlaced` is set on the first flag
  placement of a run and never cleared until `newGame` / `newGameReplay`
  / `setFixedLayout`. Removing a flag mid-game does **not** un-set it —
  otherwise a player could clear flags before the final click and game
  the bracket.
- **AutoFlag gate.** `flagAllMines()` (called from `checkWin()`) emits
  `flagToggled(true)` for every remaining mine. The `m_anyFlagPlaced`
  setter is gated `if (flagged && m_state != Won && m_state != Lost)`
  so the win-time auto-flag does not poison the bracket. Regression
  test: `testAnyFlagPlacedFalseAfterNoflagWin`.
- **Ready-state allowance.** First-click safety places no flag, so the
  bit can only flip in `Ready` (pre-first-click flag is allowed) or
  `Playing`. Won/Lost are gated; Paused is logically Playing.
- **No `recordWin` signature change.** Added a parallel
  `Stats::recordNoflagBest(name, seconds, date)` rather than threading
  a `bool noflag` through `recordWin`. Avoids touching ~10 existing
  call sites and tests; the two best-time tracks stay independent in
  storage as well (`stats/<diff>/{best_seconds,best_noflag_seconds}`).
- **Replay / Custom exclusion.** Reuses existing `m_excludeFromBest`
  flag — if it's set, neither bracket is updated.
- **Telemetry.** `game.won` event gets a `noflag=1|0` tag. No new event
  type.
- **Question marks don't count.** Cycling `None → Flag → Question`
  passes through `Flag`, which sets the bit. Cycling
  `None → Question` directly does not. Right-click cycles through Flag
  before Question, so a player who only ever uses question marks will
  still trip the bit unless they're careful — acceptable, the cycle
  was designed before the bracket existed and the bit reflects intent.
  Test: `testAnyFlagPlacedNotSetByQuestionMark` covers the direct path
  (no flag involvement).

**Risks / mitigations:**

- **QSettings schema additive.** New keys `best_noflag_seconds` /
  `best_noflag_date` default to 0 / null on legacy installs; load path
  treats absent keys as zero, so old users see "—" until they earn a
  no-flag win. Test: legacy-record-loads-as-zero.
- **Stats dialog width.** 5 columns instead of 4. Verified with
  `formatBest` lambda producing consistent "—" placeholders so the
  table doesn't ragged-edge for legacy users.
- **Translation cost.** Two new strings × 9 non-English locales =
  18 hand translations; all done in this cycle, no `tr()` literals
  left unfilled.

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
