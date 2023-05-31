cmake_minimum_required(VERSION 3.16)

project(emojirunner)

set(QT_MIN_VERSION 5.15.0)
set(KF5_MIN_VERSION 5.94.0)

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_MODULE_PATH})

# Find the required Libaries
find_package(Qt5 ${QT_MIN_VERSION} REQUIRED CONFIG COMPONENTS Widgets Core)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS I18n Runner ConfigWidgets)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDEClangFormat)
include(KDEGitCommitHooks)
include(FeatureSummary)

add_subdirectory(src)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

# CPack build
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "KRunner plugin which allows you to copy/paste emojis")
set(CPACK_PACKAGE_NAME ${CMAKE_PROJECT_NAME})
set(CPACK_PACKAGE_VERSION "2.3.0")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "alex1701c")
set(CPACK_RPM_PACKAGE_ARCHITECTURE "x86_64")
set(CPACK_RPM_PACKAGE_LICENSE "LGPL 3")

# Then names of dependencies and paths might differ
if (CPACK_DEBIAN) # -DCPACK_DEBIAN=true -DKDE_INSTALL_QTPLUGINDIR=/usr/lib/x86_64-linux-gnu/qt5/plugins
	set(CPACK_DEBIAN_PACKAGE_DEPENDS "libkf5runner5 (>= ${KF5Runner_VERSION}), libxdo3 (>= 3.20160805.1)")
	set(CPACK_DEBIAN_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-Debian.deb")
	set(CPACK_GENERATOR "DEB")
elseif(CPACK_FEDORA) # -DCPACK_FEDORA=true -DKDE_INSTALL_QTPLUGINDIR=/usr/lib64/qt5/plugins
	set(CPACK_RPM_PACKAGE_REQUIRES "kf5-krunner >= ${KF5Runner_VERSION}; libxdo3 >= 3.20160805.1")
	set(CPACK_RPM_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-Fedora.rpm")
	set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "/usr/lib64/qt5;/usr/lib64/qt5/plugins;/usr/lib64/qt5/plugins/kf5;/usr/lib64/qt5/plugins/kf5/krunner;/usr/share/kservices5;/usr/share/icons")
	set(CPACK_GENERATOR "RPM")
elseif(CPACK_OPENSUSE) # -DCPACK_OPENSUSE=true -DKDE_INSTALL_QTPLUGINDIR=/usr/lib64/qt5/plugins
	set(CPACK_RPM_PACKAGE_REQUIRES "libKF5Runner5 >= ${KF5Runner_VERSION}")
	set(CPACK_RPM_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-OpenSUSE.rpm")
	set(CPACK_GENERATOR "RPM")
endif()
# We always have to explicitely set the generator when using cpack, otherwise
# the different paths of the distros might get messed up
unset(CPACK_DEBIAN CACHE)
unset(CPACK_FEDORA CACHE)
unset(CPACK_OPENSUSE CACHE)



# These scripts will kill krunner and when it is invoked next time the plugins will be loaded
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/postinst;${CMAKE_CURRENT_SOURCE_DIR}/postrm")
set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/postinst")
set(CPACK_RPM_POST_UNINSTALL_SCRIPT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/postrm")

INCLUDE(CPack)

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
