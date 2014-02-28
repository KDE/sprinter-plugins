project(sprinter_org_kde_calculator)

########### next target ###############


# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS} -Wno-deprecated")


include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
add_definitions(-DQT_PLUGIN)
add_library(${PROJECT_NAME} SHARED
                calculator.cpp
                qalculate_engine.cpp)
qt5_use_modules(${PROJECT_NAME} Core Gui Network)
target_link_libraries(${PROJECT_NAME} ${QALCULATE_LIBRARIES})
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${SPRINTER_PLUGINS_PATH})
