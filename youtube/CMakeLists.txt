project(sprinter_org_kde_youtube)

add_definitions(-DQT_PLUGIN)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_library(${PROJECT_NAME} SHARED youtube.cpp)
qt5_use_modules(${PROJECT_NAME} Core Gui Network)
target_link_libraries(${PROJECT_NAME} Sprinter)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${SPRINTER_PLUGINS_PATH})
