project(sprinter_org_kde_kill)

add_definitions(-DQT_PLUGIN)
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${KSYSGUARDPROC_INCLUDE_DIRS})
add_library(${PROJECT_NAME} SHARED kill.cpp)
qt5_use_modules(${PROJECT_NAME} Core Gui)
target_link_libraries(${PROJECT_NAME} KF5::KIOWidgets KF5::I18n KF5::KIOCore KF5::Auth Sprinter ${KSYSGUARDPROC_LIBRARIES})
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${SPRINTER_PLUGINS_PATH})
