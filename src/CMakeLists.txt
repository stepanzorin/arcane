add_subdirectory(util)

target_sources(
    arcane
    PRIVATE # cmake-format: sort
            app_config.hpp
            main.cpp)
