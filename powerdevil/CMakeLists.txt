project(sprinter_org_kde_sprinter_powerdevil)

add_definitions(-DQT_PLUGIN)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_library(${PROJECT_NAME} SHARED powerdevil.cpp)
qt5_use_modules(${PROJECT_NAME} Core Gui)
target_link_libraries(${PROJECT_NAME} KF5::Solid KF5::I18n)
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${SPRINTER_PLUGINS_PATH})
