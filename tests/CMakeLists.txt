find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# ── MineButton unit tests ────────────────────────────────────────────────────
qt_add_executable(tst_minebutton tst_minebutton.cpp
    ../minebutton.cpp
    ../minefield.cpp
)
target_include_directories(tst_minebutton PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(tst_minebutton PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
)
if(ENABLE_COVERAGE)
    target_compile_options(tst_minebutton PRIVATE --coverage)
    target_link_options(tst_minebutton PRIVATE --coverage)
endif()
add_test(NAME tst_minebutton COMMAND tst_minebutton)
set_tests_properties(tst_minebutton PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

# ── MineField unit tests ─────────────────────────────────────────────────────
qt_add_executable(tst_minefield tst_minefield.cpp
    ../minebutton.cpp
    ../minefield.cpp
)
target_include_directories(tst_minefield PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(tst_minefield PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
)
if(ENABLE_COVERAGE)
    target_compile_options(tst_minefield PRIVATE --coverage)
    target_link_options(tst_minefield PRIVATE --coverage)
endif()
add_test(NAME tst_minefield COMMAND tst_minefield)
set_tests_properties(tst_minefield PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)
