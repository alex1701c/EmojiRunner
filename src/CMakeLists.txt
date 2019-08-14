add_definitions(-DTRANSLATION_DOMAIN=\"plasma_runner_org.kde.emojirunner\")

set(emojirunner_SRCS emojirunner.cpp Emoji.h EmojiCategory.h FileReader.cpp FileReader.h)

add_library(krunner_emojirunner MODULE ${emojirunner_SRCS})
target_link_libraries(krunner_emojirunner KF5::Runner KF5::I18n Qt5::Widgets)

set(kcm_krunner_emojirunner_SRCS
        config/emojirunner_config.cpp
        Emoji.h FileReader.cpp FileReader.h)

ki18n_wrap_ui(kcm_krunner_emojirunner_SRCS config/emojirunner_config.ui)
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


install(TARGETS krunner_emojirunner kcm_krunner_emojirunner DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-runner-emojirunner.desktop config/plasma-runner-emojirunner_config.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(FILES emojis.json DESTINATION $ENV{HOME}/.config/)
install(FILES emojirunner-launcher.desktop DESTINATION $ENV{HOME}/.local/share/applications/)
install(FILES paste.py DESTINATION $ENV{HOME}/.local/share/emojirunner/)
