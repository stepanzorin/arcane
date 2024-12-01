target_sources(
    arcane
    PRIVATE # cmake-format: sort
            mesh.cpp
            mesh.hpp
            primitives.hpp
            systems.cpp
            systems.hpp)

# Shader part
target_sources(
    arcane
    PRIVATE # cmake-format: sort
            shaders/draw_mesh_pipeline.cpp
            shaders/draw_mesh_pipeline.hpp)
