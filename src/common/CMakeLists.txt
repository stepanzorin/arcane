target_sources(
    arcane
    PRIVATE # cmake-format: sort
            samplers.hpp)

# shaders
target_sources(
    arcane
    PRIVATE # cmake-format: sort
            shaders/pipeline_functions.cpp
            shaders/pipeline_functions.hpp)
