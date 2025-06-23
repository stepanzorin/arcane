target_sources(
    arcane
    PRIVATE # cmake-format: sort
            common.hpp
            passes/common.hpp
            passes/deferred_shading.hpp
            passes/gbuffer.cpp
            passes/gbuffer.hpp
            passes/lighting.cpp
            passes/lighting.hpp
            passes/wireframe.cpp
            passes/wireframe.hpp)
