add_definitions(-DTRANSLATION_DOMAIN=\"plasma_runner_org.kde.emojirunner\")
set(CMAKE_AUTOUIC ON)

kcoreaddons_add_plugin(krunner_emojirunner SOURCES runner/emojirunner.cpp core/Emoji.cpp core/FileReader.cpp INSTALL_NAMESPACE "kf5/krunner")
target_link_libraries(krunner_emojirunner KF5::Runner Qt::Widgets) # Widgets for QAction, this will change to Qt::Gui in Qt6

find_path(XDO_HEADER xdo.h)
find_library(XDO_LIB "xdo")
if (NOT XDO_LIB)
    message(WARNING "Could not find libxdo-dev, proceeding with xdotool https://github.com/jordansissel/xdotool")
else ()
    add_definitions(-DXDO_LIB)
    target_include_directories(krunner_emojirunner PRIVATE ${XDO_HEADER})
    target_link_libraries(krunner_emojirunner ${XDO_LIB})
endif ()

add_library(kcm_krunner_emojirunner MODULE config/emojirunner_config.ui config/emojirunner_popup.ui)
target_sources(kcm_krunner_emojirunner PRIVATE
    config/emojirunner_config.cpp
    core/Emoji.cpp
    core/FileReader.cpp
    config/emojirunner_popup.cpp
)
target_link_libraries(kcm_krunner_emojirunner
    Qt::Core
    Qt::Gui
    KF5::CoreAddons
    KF5::ConfigCore
    KF5::ConfigWidgets
)
install(TARGETS kcm_krunner_emojirunner DESTINATION ${KDE_INSTALL_QTPLUGINDIR})
install(FILES core/emojis.json DESTINATION ${KDE_INSTALL_DATADIR}/emojirunner/)
