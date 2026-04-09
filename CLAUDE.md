# QMineSweeper

A Qt6-based Minesweeper game.

## Architecture

- **MineButton** (`minebutton.h` / `minebutton.cpp`): A single cell on the minefield. Extends `QPushButton`. Tracks whether the cell is mined, flagged, or opened and its adjacent mine count.
- **MineField** (`minefield.h` / `minefield.cpp`): A `QWidget` that contains a 15×15 grid of `MineButton`s. Responsible for placing 20 mines at random and computing adjacent mine numbers for each cell.
- **MainWindow** (`mainwindow.h` / `mainwindow.cpp`): Top-level window. Hosts `MineField` and a timer/mine-count display.

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
| `ubuntu.yml` | push / PR to `main` | Build, test, and coverage report on Ubuntu |
| `windows.yml` | push / PR to `main` | Build on Windows |
| `release.yml` | push tag `v*` | Build release artifacts for Linux & Windows and publish a GitHub Release |
| `formatter.yml` | every push | Enforce clang-format code style |
