cmake_minimum_required(VERSION 3.21)

project(QMineSweeper
    VERSION 1.10.0
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
option(ENABLE_SENTRY "Link sentry-native for crash reports and telemetry" OFF)
set(SENTRY_DSN
    "https://aa36f66967ebf04fed7f355ddbf56a22@o4509745779048448.ingest.de.sentry.io/4511265438564432"
    CACHE STRING "Public Sentry DSN embedded at build time")
set(SENTRY_NATIVE_VERSION "0.7.17" CACHE STRING "sentry-native tag to fetch")

find_package(Qt6 6.2 REQUIRED COMPONENTS Widgets LinguistTools)

qt_standard_project_setup(REQUIRES 6.2)

set(TS_FILES
    translations/QMineSweeper_en.ts
    translations/QMineSweeper_tr_TR.ts
    translations/QMineSweeper_zh_CN.ts
    translations/QMineSweeper_hi_IN.ts
    translations/QMineSweeper_es_ES.ts
    translations/QMineSweeper_ar_SA.ts
    translations/QMineSweeper_fr_FR.ts
    translations/QMineSweeper_ru_RU.ts
    translations/QMineSweeper_pt_BR.ts
    translations/QMineSweeper_de_DE.ts
)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    minefield.h
    minefield.cpp
    minebutton.cpp
    minebutton.h
    telemetry.cpp
    telemetry.h
    language.cpp
    language.h
    stats.cpp
    stats.h
    smiley.h
    tutorial.cpp
    tutorial.h
    resources.qrc
)

set(MACOSX_BUNDLE_ICON "${CMAKE_SOURCE_DIR}/icons/QMineSweeper.icns")

if(APPLE)
    set_source_files_properties("${MACOSX_BUNDLE_ICON}" PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources")
    list(APPEND PROJECT_SOURCES "${MACOSX_BUNDLE_ICON}")
endif()

if(WIN32)
    # Embed the .ico so the .exe gets the app icon in Explorer.
    file(WRITE "${CMAKE_BINARY_DIR}/qminesweeper.rc"
         "IDI_ICON1 ICON \"${CMAKE_SOURCE_DIR}/icons/QMineSweeper.ico\"\n")
    list(APPEND PROJECT_SOURCES "${CMAKE_BINARY_DIR}/qminesweeper.rc")
endif()

qt_add_executable(QMineSweeper
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

qt_add_translations(QMineSweeper TS_FILES ${TS_FILES})

target_link_libraries(QMineSweeper PRIVATE Qt6::Widgets)

target_compile_definitions(QMineSweeper PRIVATE
    QMS_VERSION="${PROJECT_VERSION}"
)

if(ENABLE_SENTRY)
    include(FetchContent)
    set(SENTRY_BACKEND inproc CACHE STRING "" FORCE)
    set(SENTRY_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(SENTRY_INTEGRATION_QT YES CACHE BOOL "" FORCE)
    FetchContent_Declare(
        sentry_native
        GIT_REPOSITORY https://github.com/getsentry/sentry-native.git
        GIT_TAG ${SENTRY_NATIVE_VERSION}
    )
    FetchContent_MakeAvailable(sentry_native)
    target_link_libraries(QMineSweeper PRIVATE sentry::sentry)
    target_compile_definitions(QMineSweeper PRIVATE
        QMS_SENTRY_ENABLED=1
        QMS_SENTRY_DSN="${SENTRY_DSN}"
    )
endif()

set_target_properties(QMineSweeper PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_GUI_IDENTIFIER com.mavrikant.qminesweeper
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE_ICON_FILE QMineSweeper.icns
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
