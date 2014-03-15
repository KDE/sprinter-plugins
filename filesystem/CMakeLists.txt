project(sprinter_org_kde_filesystem)

add_definitions(-DQT_PLUGIN)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_library(${PROJECT_NAME} SHARED filesystem.cpp)
qt5_use_modules(${PROJECT_NAME} Core Gui)
target_link_libraries(${PROJECT_NAME} KF5::KIOWidgets KF5::I18n)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${SPRINTER_PLUGINS_PATH})


# get_directory_property(DBG INCLUDE_DIRECTORIES)
# message("****************** ${DBG}")