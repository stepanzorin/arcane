target_sources(
    arcane
    PRIVATE # cmake-format: sort
            common.hpp
            passes/common.hpp
            passes/gbuffer.cpp
            passes/gbuffer.hpp
            passes/wireframe.cpp
            passes/wireframe.hpp)
