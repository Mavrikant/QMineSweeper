# QMineSweeper

[![Ubuntu Build](https://github.com/Mavrikant/QMineSweeper/actions/workflows/ubuntu.yml/badge.svg)](https://github.com/Mavrikant/QMineSweeper/actions/workflows/ubuntu.yml)
[![Windows Build](https://github.com/Mavrikant/QMineSweeper/actions/workflows/windows.yml/badge.svg)](https://github.com/Mavrikant/QMineSweeper/actions/workflows/windows.yml)
[![macOS Build](https://github.com/Mavrikant/QMineSweeper/actions/workflows/macos.yml/badge.svg)](https://github.com/Mavrikant/QMineSweeper/actions/workflows/macos.yml)
[![clang-format](https://github.com/Mavrikant/QMineSweeper/actions/workflows/formatter.yml/badge.svg)](https://github.com/Mavrikant/QMineSweeper/actions/workflows/formatter.yml)
[![Release](https://github.com/Mavrikant/QMineSweeper/actions/workflows/release.yml/badge.svg)](https://github.com/Mavrikant/QMineSweeper/actions/workflows/release.yml)
[![codecov](https://codecov.io/gh/Mavrikant/QMineSweeper/branch/main/graph/badge.svg)](https://codecov.io/gh/Mavrikant/QMineSweeper)
[![Latest Release](https://img.shields.io/github/v/release/Mavrikant/QMineSweeper?include_prereleases&sort=semver)](https://github.com/Mavrikant/QMineSweeper/releases/latest)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![CodeFactor](https://www.codefactor.io/repository/github/mavrikant/qminesweeper/badge)](https://www.codefactor.io/repository/github/mavrikant/qminesweeper)

A modern implementation of the classic **Minesweeper** game, written in **C++20** using the **Qt 6** widget toolkit.

## Screenshots

| Fresh Beginner board | Mid-game: numbers, flags, flood-fill |
|---|---|
| ![Fresh 9×9 Beginner board](docs/screenshots/01-fresh-beginner.png) | ![Mid-game with numbers and flags](docs/screenshots/02-playing.png) |

| Settings → Language (10 locales) | Game over — all mines revealed |
|---|---|
| ![Settings → Language menu showing flag-iconified picker for 10 locales](docs/screenshots/03-language-menu.png) | ![Game-over state with all mines revealed and the clicked mine highlighted in red](docs/screenshots/04-boom.png) |

## Features

- Three difficulty levels: Beginner (9×9, 10 mines), Intermediate (16×16, 40 mines), Expert (30×16, 99 mines)
- First-click safety — the first cell you click is always a zero, so you can start with a flood open
- Left-click reveals, right-click toggles a flag, middle-click (or left+right) chords to auto-reveal neighbours
- Automatic flood-fill reveal for empty regions
- Win / loss detection with end-of-game dialog and remaining-mine auto-flagging
- Real-time elapsed-time display (starts on first click) and live mine counter
- Game → New (Ctrl+N) and difficulty selection persisted via `QSettings`
- Cross-platform: Linux, Windows and macOS (universal binary on macOS)
- Unit tests powered by the Qt Test framework (36 tests covering the game-state machine, first-click safety, chord logic, etc.)
- Code coverage reports generated with `lcov` / `genhtml` on every CI run
- Opt-in anonymous crash reports and usage telemetry via [Sentry](https://sentry.io) (release builds only; disabled by default until you consent)
- **Ten built-in UI languages** with system-locale auto-detection and a flag-iconified picker under `Settings → Language`

## Download

Pre-built binaries for Linux, Windows and macOS are published with every
tagged release on the [**Releases**](https://github.com/Mavrikant/QMineSweeper/releases/latest)
page:

| Platform | Asset                                 |
|----------|---------------------------------------|
| Linux    | `QMineSweeper-linux-x86_64.AppImage`  |
| Linux    | `QMineSweeper-linux-x86_64.tar.gz`    |
| Windows  | `QMineSweeper-windows-x64.zip`        |
| macOS    | `QMineSweeper-macos-universal.dmg` (Apple Silicon + Intel) |

SHA-256 checksums for every asset are provided in `SHA256SUMS.txt`.

### Installing on macOS

The macOS build is a **universal binary** (Apple Silicon + Intel) that is
**ad-hoc code-signed** but not notarised. On first launch, macOS Gatekeeper
will show a *"QMineSweeper is damaged and can't be opened"* dialog. This
is expected — paid Apple Developer notarisation ($99/year) is out of scope
for an open-source project.

Clear the quarantine flag once and the app will open normally:

```bash
xattr -cr /Applications/QMineSweeper.app
```

This is the same workflow used by other open-source Qt/native apps
(Ladybird, 0 A.D., OpenTTD, …). You only need to do it once per install.

## Prerequisites

- Qt 6.2 or later (Widgets + LinguistTools modules)
- CMake 3.21 or later
- A C++20 capable compiler (GCC 11+, Clang 13+, MSVC 2019+)
- Ninja (recommended)

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -G Ninja
cmake --build build
```

If Qt is installed in a non-standard location (e.g. via the Qt online
installer or Homebrew) pass `CMAKE_PREFIX_PATH`:

```bash
# macOS (Homebrew)
cmake -B build \
  -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/qt \
  -DCMAKE_BUILD_TYPE=Release \
  -G Ninja

# Linux / Qt online installer
cmake -B build \
  -DCMAKE_PREFIX_PATH=/path/to/Qt/6.7.2/gcc_64 \
  -DCMAKE_BUILD_TYPE=Release \
  -G Ninja

cmake --build build
```

Run the game:

```bash
# Linux
./build/QMineSweeper

# macOS
open build/QMineSweeper.app
```

## Running the tests

Unit tests are built by default (`-DBUILD_TESTING=ON`).

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
```

On headless systems export the offscreen platform plugin before running
`ctest`:

```bash
QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure
```

## Code coverage

Pass `-DENABLE_COVERAGE=ON` (GCC/Clang only) to instrument the binaries,
then use `lcov` to generate an HTML report:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON -G Ninja
cmake --build build
QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure
lcov --capture --directory build --output-file build/coverage.info \
  --ignore-errors mismatch
lcov --remove build/coverage.info '*/Qt/*' '*/tests/*' '/usr/*' \
  --output-file build/coverage.info
genhtml build/coverage.info --output-directory build/coverage_html
```

Open `build/coverage_html/index.html` to view the report. The same
report is generated on every push to `main` by the Ubuntu CI job, uploaded
to [Codecov](https://codecov.io/gh/Mavrikant/QMineSweeper), and also
attached as an artifact to the workflow run.

## Languages

The UI ships with translations for 10 languages. On first launch the app
picks the best match for your system locale; if no match is found it falls
back to English. You can override the choice any time under
`Settings → Language` — the menu shows a country flag next to each
language, plus an "Auto (system)" entry that clears the override.
Language changes take effect after a restart (the app offers to restart
for you).

| Code    | Flag | Native name          | English              |
|---------|------|----------------------|----------------------|
| `en`    | 🇺🇸  | English              | English              |
| `tr_TR` | 🇹🇷  | Türkçe               | Turkish              |
| `zh_CN` | 🇨🇳  | 中文（简体）          | Chinese (Simplified) |
| `hi_IN` | 🇮🇳  | हिन्दी                  | Hindi                |
| `es_ES` | 🇪🇸  | Español              | Spanish              |
| `ar_SA` | 🇸🇦  | العربية               | Arabic (RTL)         |
| `fr_FR` | 🇫🇷  | Français             | French               |
| `ru_RU` | 🇷🇺  | Русский              | Russian              |
| `pt_BR` | 🇧🇷  | Português            | Portuguese (Brazil)  |
| `de_DE` | 🇩🇪  | Deutsch              | German               |

Translations live under [`translations/`](translations/) as Qt Linguist
`.ts` files. Contributions are very welcome — open a PR with an updated
`.ts` file; `qt_add_translations` in [CMakeLists.txt](CMakeLists.txt)
compiles them into embedded `.qm` resources at build time.

Flag icons are sourced from [flagcdn.com](https://flagcdn.com) (backed by
the MIT-licensed [lipis/flag-icons](https://github.com/lipis/flag-icons);
the flag imagery itself is public domain). Rerun
[`icons/flags/fetch_flags.sh`](icons/flags/fetch_flags.sh) to regenerate.

## Privacy and telemetry

Release builds include an **opt-in** integration with [Sentry](https://sentry.io)
via [`sentry-native`](https://github.com/getsentry/sentry-native). On first
launch, you are asked whether to send anonymous crash reports and usage
statistics. You can change the choice later under `Settings → Send
anonymous crash reports and usage data`.

**What gets sent (only if you opt in)**:
- Unhandled crashes (signal, stack trace, loaded modules, Qt version, OS, CPU arch)
- Game lifecycle events: `game.started`, `game.won`, `game.lost` — each tagged with difficulty and duration
- An anonymous install ID (a random UUID stored locally; does not identify you)
- Release-health session data (so we can see stability across versions)
- Approximate geo from your IP (country-level), inferred by Sentry's ingest

**What is NOT sent**:
- Your name, email, username, or IP address in event payloads
- File paths, in-game actions beyond win/loss, your play sessions' board layout
- Anything from a debug build (those are tagged `environment=debug` and discarded from release dashboards)

Local (`-DENABLE_SENTRY=OFF`, the default) and source builds have no telemetry
compiled in at all. The CI release workflow sets `-DENABLE_SENTRY=ON`.

To disable from the command line on a release build, delete the persisted
choice — Qt stores it in the platform-native settings store:

```bash
# macOS
defaults delete com.mavrikant.QMineSweeper
# Linux
rm -rf ~/.config/Mavrikant/QMineSweeper.conf
# Windows: registry under HKCU\Software\Mavrikant\QMineSweeper
```

The app will re-ask on next launch.

## Code style

Formatting is enforced by [`clang-format`](https://clang.llvm.org/docs/ClangFormat.html)
and validated on every push by the `formatter.yml` workflow. Run before
committing:

```bash
clang-format -i *.cpp *.h tests/*.cpp
```

## Project layout

```
.
├── CMakeLists.txt        # Top-level build configuration
├── main.cpp              # Application entry point
├── mainwindow.{h,cpp,ui} # Top-level window, menus, timer, mine counter
├── minefield.{h,cpp}     # Grid widget, game state, mine placement
├── minebutton.{h,cpp}    # Single cell: reveals, flags, chords, signals
├── telemetry.{h,cpp}     # Sentry wrapper (no-op when ENABLE_SENTRY=OFF)
├── language.{h,cpp}      # Supported locales, startup resolver, QTranslator applier
├── translations/         # Qt Linguist .ts sources for all 10 locales
├── icons/flags/          # Country flag PNGs shown in Settings → Language
├── resources.qrc         # Embedded icons (red flag, explosion)
├── icons/                # PNG assets
├── tests/                # Qt Test unit tests
└── .github/workflows/    # CI: Ubuntu, Windows, macOS, formatter, release
```

## Architecture

- **MineButton** (`minebutton.{h,cpp}`) — a single cell on the grid,
  inheriting from `QPushButton`. Owns per-cell state (mined, flagged,
  opened, number) and emits input signals (`cellPressed`, `cellOpened`,
  `explosion`, `flagToggled`, `chordRequested`, `checkNeighbours`).
- **MineField** (`minefield.{h,cpp}`) — the grid widget and the game-logic
  owner. Holds a `GameState { Ready, Playing, Won, Lost }`, defers mine
  placement until the first click (first-click safety), runs flood-fill,
  detects win / loss, and emits `gameStarted` / `gameWon` / `gameLost` /
  `mineCountChanged` up to the main window.
- **MainWindow** (`mainwindow.{h,cpp,ui}`) — presentation only. Menus,
  timer display, end-of-game `QMessageBox`, difficulty persistence via
  `QSettings`.

## Continuous integration

| Workflow       | Trigger                   | Description                                                               |
|----------------|---------------------------|---------------------------------------------------------------------------|
| `ubuntu.yml`   | push / PR to `main`       | Build, test, `lcov` coverage report + Codecov upload + HTML artifact      |
| `windows.yml`  | push / PR to `main`       | Build and test on Windows                                                 |
| `macos.yml`    | push / PR to `main`       | Build and test on macOS (Apple Silicon)                                   |
| `formatter.yml`| push / PR to `main`       | Enforce `clang-format` code style                                         |
| `release.yml`  | push tag `v*` / manual    | Build Linux, Windows and **universal ad-hoc-signed macOS** binaries; publish a GitHub Release with SHA-256 sums |

## Releasing

Tag a commit with a `v*` tag and push the tag:

```bash
git tag -a v1.0.0 -m "QMineSweeper 1.0.0"
git push origin v1.0.0
```

The `release.yml` workflow will build binaries for Linux, Windows and
macOS, ad-hoc codesign the macOS bundle, generate SHA-256 checksums, and
publish everything as assets on a new GitHub Release. The workflow can
also be triggered manually via `workflow_dispatch` from the Actions tab.

## License

This project is licensed under the terms of the
[MIT License](https://choosealicense.com/licenses/mit/).

Copyright (c) 2020 M. Serdar Karaman
