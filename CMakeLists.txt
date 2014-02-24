find_package(KF5DBusAddons)
find_package(KF5Activities)
find_package(Qalculate)

if (QALCULATE_FOUND)
    add_subdirectory(calculator)
endif (QALCULATE_FOUND)

if (KF5Activities_FOUND)
    add_subdirectory(activities)
endif (KF5Activities_FOUND)

add_subdirectory(c)
add_subdirectory(datetime)
add_subdirectory(youtube)
