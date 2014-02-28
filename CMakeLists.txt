cmake_minimum_required(VERSION 2.8)

# ECM setup
find_package(ECM 0.0.10 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH
   ${ECM_MODULE_PATH}
   ${ECM_KDE_MODULE_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
   ${CMAKE_MODULE_PATH})

include(FeatureSummary)

find_package(Sprinter REQUIRED)
#TODO: make i18n optional for runners that don't need it?
find_package(KF5 NO_MODULE
                 COMPONENTS I18n
                 OPTIONAL_COMPONENTS
                    Activities
                    CoreAddons
                    ConfigDBusAddons
                    KIO)
find_package(Qalculate)

if (NOT APPLE)
    find_package(X11)
endif()

set(HAVE_X11 ${X11_FOUND})

if(X11_FOUND)
    find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED X11Extras)
    find_package(KF5WindowSystem)
endif (X11_FOUND)

set(SPRINTER_PLUGINS_PATH plugins/sprinter)
set(CMAKE_AUTOMOC ON)
add_definitions("-Wall -std=c++0x")
include_directories(${CMAKE_INSTALL_PREFIX}/include/sprinter)

###
### add the runners below, depending on the packages found above
###

add_subdirectory(c)
add_subdirectory(datetime)
add_subdirectory(youtube)

if (QALCULATE_FOUND)
    add_subdirectory(calculator)
endif (QALCULATE_FOUND)

if (KF5Activities_FOUND)
    add_subdirectory(activities)
endif (KF5Activities_FOUND)

if (KF5KIO_FOUND AND KF5CoreAddons_FOUND AND KF5Config_FOUND)
#     currently this guy crashes somewhere in KIO
#     add_subdirectory(uri)
    add_subdirectory(recentdocs)
endif (KF5KIO_FOUND)

if (KF5WindowSystem_FOUND)
    # TODO: needs porting to non-x11. uses NETRootInfo and QX11Info
#     add_subdirectory(windows)
endif (KF5WindowSystem_FOUND)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
