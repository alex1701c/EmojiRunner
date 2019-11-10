find_package(Qt5Test REQUIRED)
enable_testing()

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(performancetest_includes_SRCS
        FileReaderBenchmarker.h
        ../EmojiCategory.h
        ../Emoji.h
        ../FileReader.cpp
        )

add_executable(performancetests ${performancetest_includes_SRCS})
add_test(performancetests performancetests)
target_link_libraries(performancetests
        Qt5::Test
        Qt5::Core
        Qt5::Gui
        KF5::CoreAddons
        KF5::ConfigCore
        KF5::I18n
        KF5::ConfigWidgets
        KF5::Runner
        )
