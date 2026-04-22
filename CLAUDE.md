# QMineSweeper

A Qt6-based Minesweeper game with Beginner / Intermediate / Expert difficulty, first-click safety, chord click, and a proper win / loss state machine.

## Architecture

- **MineButton** (`minebutton.h` / `minebutton.cpp`): A single cell on the minefield, extending `QPushButton`. Owns per-cell state (mined, flagged, opened, adjacent-mine count) and emits input signals — `cellPressed`, `cellOpened`, `explosion`, `flagToggled`, `chordRequested`, `checkNeighbours`. No back-pointer to `MineField`: signals up, slots down.
- **MineField** (`minefield.h` / `minefield.cpp`): The grid widget and game-logic owner. Holds `GameState { Ready, Playing, Won, Lost }`, defers mine placement until the first click (the pressed cell and its 8 neighbours are excluded, guaranteeing a zero-start), runs flood-fill, detects win / loss, and emits `gameStarted` / `gameWon` / `gameLost` / `mineCountChanged` upward. Grid size and mine count are runtime-configurable via `newGame(Difficulty)`; presets: `Beginner` (9×9, 10), `Intermediate` (16×16, 40), `Expert` (30×16, 99).
- **MainWindow** (`mainwindow.h` / `mainwindow.cpp` / `mainwindow.ui`): Presentation only. Menus (`Game → New`, `Game → Difficulty`, `Game → Quit`), timer tied to `gameStarted` / `gameWon` / `gameLost`, end-of-game `QMessageBox`, and difficulty persistence via `QSettings`.

## Prerequisites

- Qt 6.2 or later (Widgets + LinguistTools modules)
- CMake ≥ 3.5
- C++20-capable compiler (GCC 11+, Clang 13+, MSVC 2019+)
- Ninja (recommended) or Make / NMake

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

### macOS signing policy

Release builds are **ad-hoc code-signed** (`codesign -s -`) so they run on Apple Silicon without a paid Apple Developer account. They are **not notarised**, so users must clear quarantine on first launch: `xattr -cr /Applications/QMineSweeper.app`. Do not add notarisation unless a Developer ID certificate is available — it requires `$99/year` and is out of scope for this project.
