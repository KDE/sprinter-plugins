set(pluginInfoGenerator_SRCS
    pluginInfoGenerator.cc
)
add_executable(pluginInfoGenerator ${pluginInfoGenerator_SRCS})
qt5_use_modules(pluginInfoGenerator Core)
