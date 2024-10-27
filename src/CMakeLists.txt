add_subdirectory(common)
add_subdirectory(util)
add_subdirectory(vulkan)

target_sources(
    arcane
    PRIVATE # cmake-format: sort
            app_config.cpp
            app_config.hpp
            application.cpp
            application.hpp
            main.cpp
            os.h
            window.cpp
            window.hpp
            window_config.cpp
            window_config.hpp
            window_glfw.cpp)
