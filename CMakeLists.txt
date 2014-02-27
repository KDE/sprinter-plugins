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
find_package(KF5DBusAddons)
find_package(KF5KIO)
find_package(KF5Activities)
find_package(KF5CoreAddons)
find_package(KF5Config)
find_package(Qalculate)

set(SPRINTER_PLUGINS_PATH plugins/sprinter)
set(CMAKE_AUTOMOC ON)
add_definitions("-Wall -std=c++0x")

if (QALCULATE_FOUND)
    add_subdirectory(calculator)
endif (QALCULATE_FOUND)

if (KF5Activities_FOUND)
    add_subdirectory(activities)
endif (KF5Activities_FOUND)

if (KF5KIO_FOUND)
#     currently this guy crashes somewhere in KIO
#     add_subdirectory(uri)
    add_subdirectory(recentdocs)
endif (KF5KIO_FOUND)

add_subdirectory(c)
add_subdirectory(datetime)
add_subdirectory(youtube)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
