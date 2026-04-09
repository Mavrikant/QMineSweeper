# CLAUDE.md

This file provides guidance for AI coding assistants (Claude Code and others) working on this repository.

## Project Overview

**QMineSweeper** is a classic MineSweeper game built with **Qt6** and **C++20**. The game features a 15×15 grid with 20 randomly placed mines, a flag system, and an elapsed-time display.

- **Language**: C++20
- **GUI Framework**: Qt6 (Widgets)
- **Build System**: CMake 3.5+
- **Supported Platforms**: Linux, Windows

## Repository Structure

```
QMineSweeper/
├── main.cpp              # Entry point — initializes QApplication, loads translation, shows MainWindow
├── mainwindow.cpp/h      # Main window with timer (50 ms tick) and mine-count label
├── mainwindow.ui         # Qt Designer UI file
├── minefield.cpp/h       # MineField widget — 15×15 grid, mine placement, number calculation
├── minebutton.cpp/h      # MineButton (QPushButton subclass) — per-cell state and interaction
├── resources.qrc         # Embedded icons (explosion.png, redflag.png)
├── icons/                # Source icon files
├── tests/                # Unit tests (Qt Test framework)
│   ├── CMakeLists.txt
│   ├── tst_minefield.cpp
│   └── tst_minebutton.cpp
└── .github/workflows/    # CI/CD pipelines
    ├── ubuntu.yml        # Build + test + coverage (Linux)
    ├── windows.yml       # Build only (Windows)
    ├── formatter.yml     # clang-format enforcement
    └── release.yml       # Automated GitHub releases on version tags
```

## Architecture

```
MainWindow
  └── MineField (QWidget)          — manages the 15×15 grid
        └── MineButton[15][15]     — each cell (QPushButton subclass)
```

- **MineField** owns all buttons, places mines randomly with `fillMines()`, calculates adjacent mine counts with `fillNumbers()`, and recursively reveals empty regions via `checkNeighbours()`.
- **MineButton** handles left-click (open) and right-click (flag/unflag) interactions, emits `checkNeighbours` (cascade) or `explosion` (mine hit) signals.
- **MainWindow** connects signals to game-over and timer logic.

## Build Requirements

- CMake 3.5+
- Qt 6.2+ (Widgets module; Test module for tests)
- GCC 11+ (Linux) or MSVC 2019+ (Windows)
- Ninja (optional, recommended for faster builds)

## Common Commands

### Standard build (Release)
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -G Ninja
cmake --build build
./build/QMineSweeper
```

### Build with unit tests
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON -G Ninja
cmake --build build
cd build && QPA_PLATFORM=offscreen ctest --output-on-failure
```

### Build with tests and coverage report
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON -DCOVERAGE=ON -G Ninja
cmake --build build
cd build && QPA_PLATFORM=offscreen ctest --output-on-failure
lcov --capture --directory . --output-file coverage.info --gcov-tool gcov-11
lcov --remove coverage.info '/usr/*' '*/tests/*' '*/Qt*' --output-file coverage.info
genhtml coverage.info --output-directory coverage_html
# Open coverage_html/index.html in a browser
```

### Format code
```bash
clang-format -i *.cpp *.h
```

## CMake Options

| Option          | Default | Description                                |
|-----------------|---------|--------------------------------------------|
| `BUILD_TESTS`   | `OFF`   | Build the Qt Test unit test executables    |
| `COVERAGE`      | `OFF`   | Add `--coverage` flags for gcov/lcov       |
| `CMAKE_BUILD_TYPE` | —    | Use `Debug` for tests/coverage, `Release` for production |

## Code Style

- **Formatter**: clang-format (LLVM style)
- **Indent**: 4 spaces (no tabs)
- **Column limit**: 240 characters
- **Braces**: Allman style
- CI enforces formatting on every push via `formatter.yml`.
- Run `clang-format -i *.cpp *.h` before committing.

## Testing

Tests live in `tests/` and use the **Qt Test** framework (`QTest`).

- `tst_minefield` — tests MineField grid dimensions, mine count, number consistency, flag count
- `tst_minebutton` — tests MineButton state (mined, opened, flagged), number storage, signals

Tests require `QPA_PLATFORM=offscreen` in headless environments (CI, SSH sessions without a display).

## CI/CD Workflows

| Workflow         | Trigger                        | What it does                                     |
|------------------|--------------------------------|--------------------------------------------------|
| `ubuntu.yml`     | Push / PR to any branch        | Build (Debug) + run tests + generate coverage    |
| `windows.yml`    | Push / PR to main              | Build (Release) only                             |
| `formatter.yml`  | Every push                     | Check clang-format compliance                    |
| `release.yml`    | Push of `v*` tag               | Build both platforms, create GitHub Release      |

## Development Branch

Active development for docs/tests/CI happens on:
```
claude/add-docs-tests-cicd-H45cs
```

Use `git push -u origin claude/add-docs-tests-cicd-H45cs` to push changes.
