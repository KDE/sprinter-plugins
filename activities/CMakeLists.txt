project(sprinter_org_kde_sprinter_activities)

include_directories(${CMAKE_CURRENT_BINARY_DIR} KF5::Activities)
add_definitions(-DQT_PLUGIN)
add_library(${PROJECT_NAME} SHARED activities.cpp)
qt5_use_modules(${PROJECT_NAME} Core Gui DBus)
target_link_libraries(${PROJECT_NAME} KF5::Activities)
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${SPRINTER_PLUGINS_PATH})
