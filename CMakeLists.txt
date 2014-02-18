find_package(KF5DBusAddons)
find_package(KF5Activities)

if (KF5Activities_FOUND)
    add_subdirectory(activities)
endif (KF5Activities_FOUND)

add_subdirectory(c)
add_subdirectory(datetime)
add_subdirectory(youtube)
