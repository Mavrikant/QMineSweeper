find_package(Qt6 REQUIRED COMPONENTS Test Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Shared core library reused by all unit test targets.
add_library(QMineSweeperCore STATIC
    ../minebutton.cpp
    ../minefield.cpp
    ../stats.cpp
    ../tutorial.cpp
)
target_include_directories(QMineSweeperCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(QMineSweeperCore PUBLIC Qt6::Widgets)

if(ENABLE_COVERAGE)
    target_compile_options(QMineSweeperCore PRIVATE --coverage -O0 -g)
    target_link_options(QMineSweeperCore PUBLIC --coverage)
endif()

function(qms_add_test name)
    qt_add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PRIVATE
        QMineSweeperCore
        Qt6::Test
        Qt6::Widgets
    )
    if(ENABLE_COVERAGE)
        target_compile_options(${name} PRIVATE --coverage -O0 -g)
        target_link_options(${name} PRIVATE --coverage)
    endif()
    add_test(NAME ${name} COMMAND ${name})
    set_tests_properties(${name} PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    )
endfunction()

qms_add_test(tst_minebutton)
qms_add_test(tst_minefield)
qms_add_test(tst_stats)
qms_add_test(tst_smiley)
qms_add_test(tst_time_format)
qms_add_test(tst_tutorial)
qms_add_test(tst_bv_per_second_format)
qms_add_test(tst_flag_accuracy_format)
qms_add_test(tst_average_time_format)
