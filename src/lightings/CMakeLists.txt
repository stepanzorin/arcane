target_sources(
    arcane
    PRIVATE # cmake-format: sort
            systems.cpp
            systems.hpp)

# Shader part
target_sources(
    arcane
    PRIVATE # cmake-format: sort
            shaders/draw_point_light_pipeline.cpp
            shaders/draw_point_light_pipeline.hpp)
