cmake_minimum_required(VERSION 3.29)

project(
    arcane
    VERSION 1.0.1
    LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 23)

add_executable(arcane src/main.cpp)

target_compile_definitions(arcane PRIVATE SM_ARCANE_PROJECT_NAME="${CMAKE_PROJECT_NAME}")
target_compile_definitions(
    arcane
    PRIVATE SM_ARCANE_PROJECT_VERSION_MAJOR=${CMAKE_PROJECT_VERSION_MAJOR}
            SM_ARCANE_PROJECT_VERSION_MINOR=${CMAKE_PROJECT_VERSION_MINOR}
            SM_ARCANE_PROJECT_VERSION_PATCH=${CMAKE_PROJECT_VERSION_PATCH})

target_compile_definitions(arcane PRIVATE SM_ARCANE_APP_CONFIG_PATH="${CMAKE_SOURCE_DIR}/config.json")

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
