project(sprinter_org_kde_applications)

add_definitions(-DQT_PLUGIN)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_library(${PROJECT_NAME} SHARED applications.cpp)
qt5_use_modules(${PROJECT_NAME} Core Gui)
target_link_libraries(${PROJECT_NAME} Sprinter KF5::I18n KF5::Service KF5::KIOWidgets)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${SPRINTER_PLUGINS_PATH})
