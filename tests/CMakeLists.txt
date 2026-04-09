cmake_minimum_required(VERSION 3.5)

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Test Widgets REQUIRED)

# ── MineField tests ──────────────────────────────────────────────────────────
add_executable(tst_minefield
    tst_minefield.cpp
    ../minefield.cpp
    ../minebutton.cpp
    ../resources.qrc
)
target_include_directories(tst_minefield PRIVATE ..)
target_link_libraries(tst_minefield PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
)
add_test(NAME tst_minefield COMMAND tst_minefield)
set_tests_properties(tst_minefield PROPERTIES ENVIRONMENT "QPA_PLATFORM=offscreen")

# ── MineButton tests ─────────────────────────────────────────────────────────
add_executable(tst_minebutton
    tst_minebutton.cpp
    ../minefield.cpp
    ../minebutton.cpp
    ../resources.qrc
)
target_include_directories(tst_minebutton PRIVATE ..)
target_link_libraries(tst_minebutton PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
)
add_test(NAME tst_minebutton COMMAND tst_minebutton)
set_tests_properties(tst_minebutton PROPERTIES ENVIRONMENT "QPA_PLATFORM=offscreen")

# ── Coverage instrumentation (GCC only) ─────────────────────────────────────
if(COVERAGE)
    foreach(target tst_minefield tst_minebutton)
        target_compile_options(${target} PRIVATE --coverage -O0 -g)
        target_link_libraries(${target} PRIVATE --coverage)
    endforeach()
endif()
