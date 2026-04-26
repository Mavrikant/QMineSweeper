# QMineSweeper

A Qt6-based Minesweeper game with Beginner / Intermediate / Expert difficulty, first-click safety, chord click, and a proper win / loss state machine.

> **Developed by AI.** This codebase is end-to-end AI-authored — every
> source file, unit test, translation, commit message, release note, and
> doc page is drafted by Claude under human supervision. The
> About-dialog, README and translations all surface this. When you ship
> a change, the credit goes on the AI; the responsibility for shipping
> stays with the human maintainer. Treat the AI-developed framing as a
> first-class product attribute (it's in the About dialog and is a
> translated string in all 10 locales) — do not accidentally drop it
> when refactoring the About body or the README intro.

## Architecture

- **MineButton** (`minebutton.h` / `minebutton.cpp`): A single cell on the minefield, extending `QPushButton`. Owns per-cell state (mined, flagged, opened, adjacent-mine count) and emits input signals — `cellPressed`, `cellOpened`, `explosion`, `flagToggled`, `chordRequested`, `checkNeighbours`. No back-pointer to `MineField`: signals up, slots down.
- **MineField** (`minefield.h` / `minefield.cpp`): The grid widget and game-logic owner. Holds `GameState { Ready, Playing, Won, Lost }`, defers mine placement until the first click (the pressed cell and its 8 neighbours are excluded, guaranteeing a zero-start), runs flood-fill, detects win / loss, and emits `gameStarted` / `gameWon` / `gameLost` / `mineCountChanged` upward. Grid size and mine count are runtime-configurable via `newGame(Difficulty)`; presets: `Beginner` (9×9, 10), `Intermediate` (16×16, 40), `Expert` (30×16, 99).
- **MainWindow** (`mainwindow.h` / `mainwindow.cpp` / `mainwindow.ui`): Presentation only. Menus (`Game → New`, `Game → Difficulty`, `Game → Quit`, `Settings → Send crash reports`), timer tied to `gameStarted` / `gameWon` / `gameLost`, end-of-game `QMessageBox`, and difficulty persistence via `QSettings`. First-launch consent dialog for telemetry.
- **Telemetry** (`telemetry.h` / `telemetry.cpp`): Thin `sentry-native` wrapper. No-op at link time when `ENABLE_SENTRY=OFF` (default). When on, initialises Sentry after user consent, sets an anonymous install UUID as the user id, tags `os`/`arch`/`qt_version`, starts a release-health session, and exposes `recordEvent`/`addBreadcrumb` for game events (`game.started`, `game.won`, `game.lost` with difficulty + duration tags).
- **Language** (`language.h` / `language.cpp`): Owns the list of 10 supported UI locales (en, tr_TR, zh_CN, hi_IN, es_ES, ar_SA, fr_FR, ru_RU, pt_BR, de_DE), resolves the startup locale (QSettings override → `QLocale::system().uiLanguages()` → `"en"`), and installs the matching `QTranslator` loaded from the `:/i18n/` resource prefix baked by `qt_add_translations`. Translations live in `translations/QMineSweeper_<code>.ts`; flag icons in `icons/flags/<code>.png`. Language changes require a restart; `MainWindow::restartApp()` does a detached self-relaunch.
- **Stats** (`stats.h` / `stats.cpp`): Per-difficulty lifetime record (Played / Won / Best-seconds), persisted in `QSettings` under `stats/<DifficultyName>/{played,won,best_seconds}`. `recordLoss()` / `recordWin(seconds)` are called from `MainWindow::onGameLost` / `onGameWon`; `recordWin` returns `true` when the run beat the previous best, which `MainWindow` surfaces as a 🏆 "New record!" flair on the end-of-game dialog. `Game → Statistics…` opens a `QDialog` with a 3-row table plus a "Reset all" button.
- **Cell marker** — `MineButton` owns a tri-state `CellMarker { None, Flag, Question }`. Right-click cycles `None → Flag → Question → None`. Flags block reveal (left-click and flood-fill); question marks are visual annotations only (they get cleared when the cell is actually opened). Chord click counts only real flags — question marks do not satisfy a number.
- **Tutorial** (`tutorial.h` / `tutorial.cpp`): First-run walkthrough. `Tutorial::steps()` returns a static list of `QT_TR_NOOP`-marked `{title, body}` pairs. `TutorialDialog` is a modal QDialog with Back / Next / Skip and a "Step N of M" indicator; emits `completed` on Finish and `skipped` otherwise. Completion persists in `QSettings` under `tutorial/completed`. `MainWindow` checks the flag after the telemetry-consent path in its ctor and queues `showTutorialDialog()` via `QTimer::singleShot(0, …)` so the main window paints first. `Help → Tutorial` re-opens the dialog any time, regardless of the flag. Telemetry: `tutorial.completed` / `tutorial.skipped` events fire once per decision.

## Prerequisites

- Qt 6.2 or later (Widgets + LinguistTools modules). CI pins **6.7.2**; dev mac uses Homebrew `qt` (6.11+) at `/opt/homebrew/opt/qt`.
- CMake ≥ 3.21
- C++20-capable compiler (GCC 11+, Clang 13+, MSVC 2019+)
- Ninja (recommended) or Make / NMake
- For `-DENABLE_SENTRY=ON` on Linux: `libcurl4-openssl-dev`, `libssl-dev`, `zlib1g-dev` (release workflow installs them; main CI doesn't link Sentry)

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -G Ninja
cmake --build build
```

On Linux you may need to specify the Qt prefix:

```bash
cmake -B build -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x.x/gcc_64 -DCMAKE_BUILD_TYPE=Release -G Ninja
cmake --build build
```

## Run Tests

Tests use the [Qt Test](https://doc.qt.io/qt-6/qttest-index.html) framework and are built alongside the main application.

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug -G Ninja
cmake --build build
cd build
QT_QPA_PLATFORM=offscreen ctest --output-on-failure
```

## Generate Coverage Report

Pass `-DENABLE_COVERAGE=ON` when configuring (requires GCC with gcov / lcov):

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON -G Ninja
cmake --build build
cd build
QT_QPA_PLATFORM=offscreen ctest --output-on-failure
lcov --capture --directory . --output-file coverage.info --ignore-errors mismatch
lcov --remove coverage.info '*/Qt/*' '*/tests/*' '/usr/*' --output-file coverage.info
genhtml coverage.info --output-directory coverage_html
```

Open `build/coverage_html/index.html` in a browser to view the report.

## Code Style

Formatting is enforced by [clang-format](https://clang.llvm.org/docs/ClangFormat.html). Run before committing:

```bash
clang-format -i *.cpp *.h
```

The CI formatter workflow will flag any style violations.

## CI/CD

| Workflow | Trigger | Description |
|---|---|---|
| `ubuntu.yml` | push / PR to `main` | Build, test, lcov coverage + Codecov upload + HTML artifact |
| `windows.yml` | push / PR to `main` | Build and test on Windows |
| `macos.yml` | push / PR to `main` | Build and test on macOS (Apple Silicon) |
| `release.yml` | push tag `v*` / manual | Build Linux + Windows + universal ad-hoc-signed macOS binaries and publish a GitHub Release |
| `formatter.yml` | push / PR to `main` | Enforce clang-format code style |

### Telemetry (Sentry)

Release binaries are built with `-DENABLE_SENTRY=ON`. The [`sentry-native`](https://github.com/getsentry/sentry-native) SDK is fetched at configure time via `FetchContent`, built statically with `SENTRY_BACKEND=inproc` (no separate `crashpad_handler` to ship), and linked in. Inproc catches `SIGSEGV`/`SIGABRT`/`SIGBUS`/`SIGFPE`/`SIGILL` — not `SIGTERM` — which is fine for crash reporting on normal exits.

Telemetry is **opt-in**. The first launch asks. The Sentry project is `karaman/qminesweeper` on `https://karaman.sentry.io` (region `de.sentry.io`). The public DSN is hardcoded in `CMakeLists.txt`; client DSNs are safe to embed because they only grant ingest, not read.

Dev builds (`ENABLE_SENTRY=OFF`, the default) have no Sentry code linked at all — useful for fast iteration and so PRs on `main` CI stay fast.

### macOS signing policy

Release builds are **ad-hoc code-signed** (`codesign -s -`) so they run on Apple Silicon without a paid Apple Developer account. They are **not notarised**, so users must clear quarantine on first launch: `xattr -cr /Applications/QMineSweeper.app`. Do not add notarisation unless a Developer ID certificate is available — it requires `$99/year` and is out of scope for this project.

## Release process

Pre-flight: `main` CI is green, `ctest` passes locally, `clang-format --dry-run --Werror *.cpp *.h tests/*.cpp` clean.

```bash
# 1. Bump the version in CMakeLists.txt (project(QMineSweeper VERSION X.Y.Z)).
# 2. Commit the bump.
git add CMakeLists.txt
git commit -m "chore: bump version to X.Y.Z for release"
# 3. Push main so CI validates the bump commit first.
git push origin main
# 4. Annotate and push the tag — this triggers release.yml.
git tag -a vX.Y.Z -m "QMineSweeper X.Y.Z

<highlights>"
git push origin vX.Y.Z
# 5. Poll the run; when it finishes, replace the auto-generated release
#    body with a hand-written changelog (GitHub Releases → Edit).
```

Release workflow builds all three platforms in parallel, ad-hoc signs the macOS bundle, and publishes a GitHub Release with an `SHA256SUMS.txt` file. Run time ~10 min. If Linux fails at Configure CMake the first question is whether the apt-get list is missing a dev header for a new transitive dep — see Gotchas below.

## Translation workflow

All user-facing strings must go through `tr()`. Any string assembled from a runtime `const char*` — e.g. a menu label built from a `struct { const char* label; }` array — must wrap the literal in `QT_TR_NOOP("…")` so `lupdate` can extract it; the runtime lookup stays `tr(var)`.

Flow for a new string:

1. Edit source with `tr("new string")` (or `QT_TR_NOOP` + `tr(var)` at the use site).
2. `cmake --build build --target update_translations` runs `lupdate` over every `.ts` file; new sources show up with `type="unfinished"`.
3. Edit `translations/apply_translations.py` to add the new key to every per-language dict. `QMineSweeper`, `10`, `000.0` are in `NO_TRANSLATE` and intentionally left empty so Qt falls back to the source.
4. `python3 translations/apply_translations.py` rewrites each `.ts` with the applied translations.
5. `cmake --build build` rebuilds the `.qm` files via `qt_add_translations` and embeds them under `:/i18n/`.

Commit the `.ts` files and `apply_translations.py`; the `.qm` files are build artefacts and never committed.

## Gotchas

- **`qt_add_translations` vs `qt_create_translation`**: always use `qt_add_translations`. It auto-embeds `.qm` into the target under `:/i18n/`. `qt_create_translation` (the old API) does not, so any code expecting `:/i18n/QMineSweeper_xx.qm` silently fails. The v1.0→v1.1 refactor fixed this bug.
- **macOS ad-hoc codesign failures after `macdeployqt`**: the deploy step leaves read-only bits and occasional `com.apple.FinderInfo` xattrs that break `codesign`. Always `chmod -R u+w` + `xattr -cr` on the bundle *before* `codesign --deep --force -s -`.
- **Linux release needs libcurl-dev when `ENABLE_SENTRY=ON`**: sentry-native's CMake does `find_package(CURL REQUIRED)`. Ubuntu runners don't have libcurl headers by default — `release.yml` explicitly installs `libcurl4-openssl-dev`, `libssl-dev`, `zlib1g-dev`.
- **`lupdate` can't see `tr(var)`**: it's a static parser; any literal must appear directly inside `tr("…")` or be marked with `QT_TR_NOOP("…")`.
- **AppleScript/System Events on macOS**: needs Accessibility permission, which is not grantable programmatically. For any UI-driving task (screenshots, clicks), use the `computer-use` MCP instead — it uses the system CGEvent path.
- **Screencapture of covered windows**: `screencapture -l <windowid>` works regardless of z-order. Get the window ID via `CGWindowListCopyWindowInfo`; the precompiled `/tmp/winid` helper (Swift source at `/tmp/winid.swift`) prints `<id>\t<title>\tx,y,w,h` for each visible QMineSweeper window.
- **QSettings path on macOS**: uses `OrganizationDomain reversed + ApplicationName`. For this app that is `~/Library/Preferences/com.mavrikant.QMineSweeper.plist` (exact case). To flip the telemetry toggle for testing: `defaults write com.mavrikant.QMineSweeper "telemetry/enabled" -bool YES`.
- **Sentry DSN is public**: client DSNs are ingest-only and safe to embed. Spike Protection + per-DSN rate limit in the Sentry UI handle quota abuse.
