cmake_minimum_required(VERSION 3.29)

project(
    arcane
    VERSION 1.0.1
    LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 23)

add_executable(arcane src/main.cpp)

target_compile_definitions(arcane PRIVATE $<$<CONFIG:Debug>:SM_ARCANE_DEBUG_MODE>)
option(SM_ARCANE_DEBUG_MODE "Enable debugging" OFF)
if (SM_ARCANE_DEBUG_MODE)
    target_compile_definitions(arcane PRIVATE SM_ARCANE_DEBUG_MODE=1)
endif ()

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions(arcane PRIVATE VK_USE_PLATFORM_WIN32_KHR=1 NOMINMAX)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_definitions(arcane PRIVATE VK_USE_PLATFORM_XCB_KHR=1)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_compile_definitions(arcane PRIVATE VK_USE_PLATFORM_MACOS_MVK=1)
endif ()

target_compile_definitions(arcane PRIVATE SM_ARCANE_PROJECT_NAME="${CMAKE_PROJECT_NAME}")
target_compile_definitions(
    arcane
    PRIVATE SM_ARCANE_PROJECT_VERSION_MAJOR=${CMAKE_PROJECT_VERSION_MAJOR}
            SM_ARCANE_PROJECT_VERSION_MINOR=${CMAKE_PROJECT_VERSION_MINOR}
            SM_ARCANE_PROJECT_VERSION_PATCH=${CMAKE_PROJECT_VERSION_PATCH})
target_compile_definitions(arcane PRIVATE SM_ARCANE_APPLICATION_DIR_PATH="${PROJECT_SOURCE_DIR}")

set(GLSLC_EXECUTABLE $ENV{VK_SDK_PATH}/Bin/glslc.exe)

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

target_include_directories(arcane PUBLIC src)

add_subdirectory(src)
