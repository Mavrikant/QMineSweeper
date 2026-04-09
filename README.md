# QMineSweeper

[![Ubuntu Build](https://github.com/Mavrikant/QMineSweeper/actions/workflows/ubuntu.yml/badge.svg)](https://github.com/Mavrikant/QMineSweeper/actions/workflows/ubuntu.yml)
[![Windows Build](https://github.com/Mavrikant/QMineSweeper/actions/workflows/windows.yml/badge.svg)](https://github.com/Mavrikant/QMineSweeper/actions/workflows/windows.yml)
[![clang-format](https://github.com/Mavrikant/QMineSweeper/actions/workflows/formatter.yml/badge.svg)](https://github.com/Mavrikant/QMineSweeper/actions/workflows/formatter.yml)
[![Release](https://github.com/Mavrikant/QMineSweeper/actions/workflows/release.yml/badge.svg)](https://github.com/Mavrikant/QMineSweeper/actions/workflows/release.yml)
[![Latest Release](https://img.shields.io/github/v/release/Mavrikant/QMineSweeper?include_prereleases&sort=semver)](https://github.com/Mavrikant/QMineSweeper/releases/latest)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![CodeFactor](https://www.codefactor.io/repository/github/mavrikant/qminesweeper/badge)](https://www.codefactor.io/repository/github/mavrikant/qminesweeper)

A modern implementation of the classic **Minesweeper** game, written in
**C++20** using the **Qt 6** widget toolkit.

## Features

- 15x15 playing field with 20 randomly-placed mines
- Left-click to reveal a cell, right-click to flag it
- Automatic flood-fill reveal for empty regions
- Real-time elapsed-time display and live mine counter
- Cross-platform build: Linux, Windows and macOS
- Unit tests powered by the Qt Test framework
- Code coverage reports generated with `lcov` / `genhtml`

## Download

Pre-built binaries for Linux, Windows and macOS are published with every
tagged release on the [**Releases**](https://github.com/Mavrikant/QMineSweeper/releases/latest)
page:

| Platform | Asset                                 |
|----------|---------------------------------------|
| Linux    | `QMineSweeper-linux-x86_64.AppImage`  |
| Linux    | `QMineSweeper-linux-x86_64.tar.gz`    |
| Windows  | `QMineSweeper-windows-x64.zip`        |
| macOS    | `QMineSweeper-macos-universal.dmg`    |

SHA-256 checksums for every asset are provided in `SHA256SUMS.txt`.

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
installer) pass `CMAKE_PREFIX_PATH`:

```bash
cmake -B build \
  -DCMAKE_PREFIX_PATH=/path/to/Qt/6.7.2/gcc_64 \
  -DCMAKE_BUILD_TYPE=Release \
  -G Ninja
cmake --build build
```

Run the game:

```bash
./build/QMineSweeper
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

Open `build/coverage_html/index.html` to view the report.

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
├── mainwindow.{h,cpp,ui} # Top-level window, timer, mine counter
├── minefield.{h,cpp}     # 15x15 grid widget, mine placement, numbers
├── minebutton.{h,cpp}    # Single cell: opens, flags, emits signals
├── resources.qrc         # Embedded icons (red flag, explosion)
├── icons/                # PNG assets
├── tests/                # Qt Test unit tests
└── .github/workflows/    # CI: Ubuntu, Windows, formatter, release
```

## Architecture

- **MineButton** (`minebutton.{h,cpp}`): a single cell on the grid,
  inheriting from `QPushButton`. Tracks whether the cell is mined,
  flagged, or opened, and its adjacent mine count.
- **MineField** (`minefield.{h,cpp}`): a `QWidget` hosting a 15x15 grid
  of `MineButton`s. Places 20 mines at random and computes adjacent
  mine numbers for every cell.
- **MainWindow** (`mainwindow.{h,cpp,ui}`): top-level window, hosting
  the `MineField` widget, an elapsed-time label and a live mine
  counter.

## Continuous integration

| Workflow       | Trigger                  | Description                                                       |
|----------------|--------------------------|-------------------------------------------------------------------|
| `ubuntu.yml`   | push / PR to `main`      | Build, test and generate coverage report on Ubuntu                |
| `windows.yml`  | push / PR to `main`      | Build and test on Windows                                         |
| `formatter.yml`| push / PR to `main`      | Enforce `clang-format` code style                                 |
| `release.yml`  | push tag `v*` / manual   | Build Linux, Windows and macOS binaries and publish a GitHub Release |

## Releasing

Tag a commit with a `v*` tag and push the tag:

```bash
git tag -a v1.0.0 -m "QMineSweeper 1.0.0"
git push origin v1.0.0
```

The `release.yml` workflow will build binaries for Linux, Windows and
macOS, generate SHA-256 checksums, and publish them as assets to a new
GitHub Release. The workflow can also be triggered manually via
`workflow_dispatch` from the Actions tab.

## License

This project is licensed under the terms of the
[MIT License](https://choosealicense.com/licenses/mit/).

Copyright (c) 2020 M. Serdar Karaman
