add_subdirectory(util)

target_sources(
    arcane
    PRIVATE # cmake-format: sort
            app_config.cpp
            app_config.hpp
            application.cpp
            application.hpp
            main.cpp
            os.h
            vulkan_config.cpp
            vulkan_config.hpp
            window.cpp
            window.hpp
            window_config.cpp
            window_config.hpp
            window_glfw.cpp)
