add_subdirectory(cameras)
add_subdirectory(common)
add_subdirectory(primitive_graphics)
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
            renderer.cpp
            renderer.hpp
            window.cpp
            window.hpp
            window_config.cpp
            window_config.hpp
            window_glfw.cpp)
