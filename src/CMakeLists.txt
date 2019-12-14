add_definitions(-DTRANSLATION_DOMAIN=\"plasma_runner_org.kde.emojirunner\")

set(emojirunner_SRCS runner/emojirunner.cpp core/Emoji.cpp core/EmojiCategory.h core/FileReader.cpp)
add_library(krunner_emojirunner MODULE ${emojirunner_SRCS})


find_path(XDO_HEADER xdo.h)
find_library(XDO_LIB "xdo")
if (NOT XDO_LIB)
    message(WARNING "Could not find libxdo-dev, proceeding with xdotool https://github.com/jordansissel/xdotool")
    target_link_libraries(krunner_emojirunner KF5::Runner KF5::I18n Qt5::Widgets)
else ()
    add_definitions(-DXDO_LIB)
    target_include_directories(krunner_emojirunner PRIVATE ${XDO_HEADER})
    target_link_libraries(krunner_emojirunner KF5::Runner KF5::I18n Qt5::Widgets ${XDO_LIB})
endif ()

set(kcm_krunner_emojirunner_SRCS config/emojirunner_config.cpp core/Emoji.cpp core/FileReader.cpp config/emojirunner_popup.cpp)

#add_subdirectory(tests)
ki18n_wrap_ui(kcm_krunner_emojirunner_SRCS config/emojirunner_config.ui)
ki18n_wrap_ui(kcm_krunner_emojirunner_SRCS config/emojirunner_popup.ui)
add_library(kcm_krunner_emojirunner MODULE ${kcm_krunner_emojirunner_SRCS})
target_link_libraries(kcm_krunner_emojirunner
        Qt5::Core
        Qt5::Gui
        KF5::CoreAddons
        KF5::ConfigCore
        KF5::I18n
        KF5::ConfigWidgets
        KF5::Runner
        )

add_dependencies(krunner_emojirunner kcm_krunner_emojirunner)

set(EMOJI_FILE_INSTALL_DIR "/usr/share/emojirunner" CACHE STRING "Default installation directory")

install(TARGETS krunner_emojirunner kcm_krunner_emojirunner DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-runner-emojirunner.desktop config/plasma-runner-emojirunner_config.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
# Location gets changed for installation without admin priviledget
install(FILES core/emojis.json DESTINATION ${EMOJI_FILE_INSTALL_DIR})
