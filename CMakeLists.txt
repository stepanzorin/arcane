cmake_minimum_required(VERSION 3.29)

project(
        arcane
        VERSION 1.0.1
        LANGUAGES CXX C)

if (NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 23)
endif ()

add_executable(arcane src/main.cpp)

set(GLSLC_EXECUTABLE $ENV{VK_SDK_PATH}/Bin/glslc.exe)

target_include_directories(arcane PUBLIC src)

# [ FORWARD SORT BY PACKAGE NAME ]
find_package(Boost REQUIRED)
find_package(fmt REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(GTest REQUIRED)
find_package(range-v3 REQUIRED)
find_package(spdlog REQUIRED)
find_package(stb REQUIRED)
find_package(tinyobjloader REQUIRED)
find_package(Vulkan REQUIRED)

target_link_libraries(
        arcane
        PRIVATE # [ FORWARD SORT BY PACKAGE NAME ]
        Boost::boost
        Boost::json
        fmt::fmt
        glfw
        glm::glm
        GTest::GTest
        range-v3::range-v3
        spdlog::spdlog
        stb::stb
        tinyobjloader::tinyobjloader
        Vulkan::Vulkan)

add_subdirectory(src)