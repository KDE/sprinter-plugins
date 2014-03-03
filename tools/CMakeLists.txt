set(pluginInfoGenerator_SRCS
    pluginInfoGenerator.cpp
)
add_executable(pluginInfoGenerator ${pluginInfoGenerator_SRCS})
qt5_use_modules(pluginInfoGenerator Core)
