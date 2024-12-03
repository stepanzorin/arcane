target_sources(
    arcane
    PRIVATE # cmake-format: sort
            game_object.cpp
            game_object.hpp
            systems.cpp
            systems.hpp)

# Shader part
target_sources(
    arcane
    PRIVATE # cmake-format: sort
            shaders/draw_object_pipeline.cpp
            shaders/draw_object_pipeline.hpp)
