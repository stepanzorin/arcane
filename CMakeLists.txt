cmake_minimum_required(VERSION 3.29)

project(
    arcane
    VERSION 1.0.1
    LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 23)

add_executable(arcane src/main.cpp)

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
find_package(vulkan-memory-allocator REQUIRED)
find_package(Vulkan REQUIRED)

target_link_libraries(
    arcane
    PRIVATE # cmake-format: sort
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
            vulkan-memory-allocator::vulkan-memory-allocator
            Vulkan::Vulkan)

target_include_directories(arcane PUBLIC src)
set(SM_ARCANE_SHADER_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/src)

add_subdirectory(src)

# ==========================================================================================================
# GLSL part
# ==========================================================================================================

# 1. Turn on the GLSL compiler NB: `VK_SDK_PATH` is the environment variable
set(GLSLC_EXECUTABLE $ENV{VK_SDK_PATH}/Bin/glslc.exe)

# 1. Setting up
set(SM_ARCANE_SHADERS_OUTPUT_DIR ${CMAKE_BINARY_DIR}/share/shaders)
file(MAKE_DIRECTORY ${SM_ARCANE_SHADERS_OUTPUT_DIR})

set(SM_ARCANE_SHADER_DIRS
    # cmake-format: sort
    ${CMAKE_SOURCE_DIR}/src/lightings/shaders
    ${CMAKE_SOURCE_DIR}/src/objects/shaders
    ${CMAKE_SOURCE_DIR}/src/primitive_graphics/shaders)

set(SM_ARCANE_SHADER_SOURCES)

foreach (SM_ARCANE_SHADER_DIR IN LISTS SM_ARCANE_SHADER_DIRS)
    file(GLOB SM_ARCANE_SHADER_DIR_SOURCES ${SM_ARCANE_SHADER_DIR}/*.vert ${SM_ARCANE_SHADER_DIR}/*.frag)
    list(APPEND SM_ARCANE_SHADER_SOURCES ${SM_ARCANE_SHADER_DIR_SOURCES})
endforeach ()

target_compile_definitions(arcane PRIVATE SM_ARCANE_SPIRV_DIR_PATH="${SM_ARCANE_SHADERS_OUTPUT_DIR}/")

set(SM_ARCANE_SHADER_INCLUDE_OPTIONS "")
foreach (INC_DIR IN LISTS SM_ARCANE_SHADER_INCLUDE_DIRS)
    list(APPEND SM_ARCANE_SHADER_INCLUDE_OPTIONS "-I" "${INC_DIR}")
endforeach ()

foreach (SHADER_SOURCE IN LISTS SM_ARCANE_SHADER_SOURCES)
    get_filename_component(SHADER_NAME ${SHADER_SOURCE} NAME_WE)
    get_filename_component(SHADER_EXT ${SHADER_SOURCE} EXT)
    string(REGEX REPLACE "^\\." "" SHADER_EXT ${SHADER_EXT})
    set(SM_ARCANE_SHADER_SPV_OUTPUT ${SM_ARCANE_SHADERS_OUTPUT_DIR}/${SHADER_NAME}.${SHADER_EXT}.spv)

    file(RELATIVE_PATH RELATIVE_SHADER_SOURCE ${CMAKE_SOURCE_DIR} ${SHADER_SOURCE})

    add_custom_command(
        OUTPUT ${SM_ARCANE_SHADER_SPV_OUTPUT}
        COMMAND
            ${GLSLC_EXECUTABLE} ${SM_ARCANE_SHADER_INCLUDE_OPTIONS} ${SHADER_SOURCE} -o ${SM_ARCANE_SHADER_SPV_OUTPUT}
        COMMAND
            ${CMAKE_COMMAND} -E echo "Compiled the SPIR-V binary shader file from the shader ${RELATIVE_SHADER_SOURCE}"
        COMMAND_EXPAND_LISTS
        DEPENDS ${SHADER_SOURCE}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

    list(APPEND SM_ARCANE_SHADER_SPVS ${SM_ARCANE_SHADER_SPV_OUTPUT})

    target_sources(arcane PRIVATE ${SM_ARCANE_SHADER_SPV_OUTPUT})
endforeach ()

add_custom_target(shader_compiler DEPENDS ${SM_ARCANE_SHADER_SPVS})
add_dependencies(arcane shader_compiler)
