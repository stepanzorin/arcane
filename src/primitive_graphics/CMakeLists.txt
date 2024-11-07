target_sources(
    arcane
    PRIVATE # cmake-format: sort
            primitives.hpp)

# Shader part
target_sources(
    arcane
    PRIVATE # cmake-format: sort
            shaders/draw_mesh_pipeline.cpp
            shaders/draw_mesh_pipeline.hpp)
