cmake_minimum_required(VERSION 3.21)

project(QMineSweeper
    VERSION 1.0.0
    DESCRIPTION "A Qt6-based Minesweeper game"
    HOMEPAGE_URL "https://github.com/Mavrikant/QMineSweeper"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(ENABLE_COVERAGE "Enable gcov/lcov coverage instrumentation" OFF)
option(BUILD_TESTING "Build unit tests" ON)

find_package(Qt6 6.2 REQUIRED COMPONENTS Widgets LinguistTools)

qt_standard_project_setup(REQUIRES 6.2)

set(TS_FILES QMineSweeper_tr_TR.ts)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    minefield.h
    minefield.cpp
    minebutton.cpp
    minebutton.h
    resources.qrc
)

qt_add_executable(QMineSweeper
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

target_link_libraries(QMineSweeper PRIVATE Qt6::Widgets)

target_compile_definitions(QMineSweeper PRIVATE
    QMS_VERSION="${PROJECT_VERSION}"
)

set_target_properties(QMineSweeper PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_GUI_IDENTIFIER com.mavrikant.qminesweeper
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

if(ENABLE_COVERAGE)
    if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        message(WARNING "Coverage is only supported with GCC/Clang")
    else()
        target_compile_options(QMineSweeper PRIVATE --coverage -O0 -g)
        target_link_options(QMineSweeper PRIVATE --coverage)
    endif()
endif()

include(GNUInstallDirs)
install(TARGETS QMineSweeper
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_finalize_executable(QMineSweeper)

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
