// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>

#include <glm/fwd.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "vulkan/device.hpp"
#include "vulkan/device_memory.hpp"

namespace sm::arcane::primitive_graphics {

class Mesh {
public:
    struct simple_push_consts_data_s {
        glm::f32mat4 model_matrix = glm::f32mat4{1.0f};
        glm::f32mat4 normal_matrix = glm::f32mat4{1.0f};
    };

    struct vertex_s {
        glm::f32vec3 position;
        glm::f32vec4 color;

        [[nodiscard]] bool operator==(const vertex_s &other) const noexcept = default;
    };

    explicit Mesh(const vulkan::Device &device,
                  const vk::CommandPool &command_pool,
                  std::vector<vertex_s> &&vertices,
                  std::vector<std::uint32_t> &&indices);

    Mesh(const Mesh &) = delete;
    Mesh &operator=(const Mesh &) = delete;
    Mesh(Mesh &&) noexcept = delete;
    Mesh &operator=(Mesh &&) noexcept = delete;

    ~Mesh() = default;

    void bind(vk::CommandBuffer command_buffer) const;
    void draw(vk::CommandBuffer command_buffer) const;

    [[nodiscard]] std::uint32_t vertex_count() const noexcept;
    [[nodiscard]] const vulkan::DeviceMemoryBuffer &vertex_buffer() const noexcept;
    [[nodiscard]] std::uint32_t index_count() const noexcept;
    [[nodiscard]] const vulkan::DeviceMemoryBuffer &index_buffer() const noexcept;

protected:
    vk::Device m_device;

    std::vector<vertex_s> m_vertices = {};
    std::uint32_t m_vertex_count = 0;
    vulkan::DeviceMemoryBuffer m_vertex_buffer;

    std::vector<std::uint32_t> m_indices = {};
    bool m_exists_index_buffer = false;
    std::uint32_t m_index_count = 0;
    vulkan::DeviceMemoryBuffer m_index_buffer;
};

namespace blanks {

const std::vector<Mesh::vertex_s> cube_vertices{
        // left face (white)
        {{-0.5f, -0.5f, -0.5f}, {0.9f, 0.9f, 0.9f, 1.0f}}, // 0
        {{-0.5f, 0.5f, 0.5f}, {0.9f, 0.9f, 0.9f, 1.0f}}, // 1
        {{-0.5f, -0.5f, 0.5f}, {0.9f, 0.9f, 0.9f, 1.0f}}, // 2
        {{-0.5f, 0.5f, -0.5f}, {0.9f, 0.9f, 0.9f, 1.0f}}, // 3

        // right face (yellow)
        {{0.5f, -0.5f, -0.5f}, {0.8f, 0.8f, 0.1f, 1.0f}}, // 4
        {{0.5f, 0.5f, 0.5f}, {0.8f, 0.8f, 0.1f, 1.0f}}, // 5
        {{0.5f, -0.5f, 0.5f}, {0.8f, 0.8f, 0.1f, 1.0f}}, // 6
        {{0.5f, 0.5f, -0.5f}, {0.8f, 0.8f, 0.1f, 1.0f}}, // 7

        // top face (orange, remember y axis points down)
        {{-0.5f, -0.5f, -0.5f}, {0.9f, 0.6f, 0.1f, 1.0f}}, // 8
        {{0.5f, -0.5f, 0.5f}, {0.9f, 0.6f, 0.1f, 1.0f}}, // 9
        {{-0.5f, -0.5f, 0.5f}, {0.9f, 0.6f, 0.1f, 1.0f}}, // 10
        {{0.5f, -0.5f, -0.5f}, {0.9f, 0.6f, 0.1f, 1.0f}}, // 11

        // bottom face (red)
        {{-0.5f, 0.5f, -0.5f}, {0.8f, 0.1f, 0.1f, 1.0f}}, // 12
        {{0.5f, 0.5f, 0.5f}, {0.8f, 0.1f, 0.1f, 1.0f}}, // 13
        {{-0.5f, 0.5f, 0.5f}, {0.8f, 0.1f, 0.1f, 1.0f}}, // 14
        {{0.5f, 0.5f, -0.5f}, {0.8f, 0.1f, 0.1f, 1.0f}}, // 15

        // nose face (blue)
        {{-0.5f, -0.5f, 0.5f}, {0.1f, 0.1f, 0.8f, 1.0f}}, // 16
        {{0.5f, 0.5f, 0.5f}, {0.1f, 0.1f, 0.8f, 1.0f}}, // 17
        {{-0.5f, 0.5f, 0.5f}, {0.1f, 0.1f, 0.8f, 1.0f}}, // 18
        {{0.5f, -0.5f, 0.5f}, {0.1f, 0.1f, 0.8f, 1.0f}}, // 19

        // tail face (green)
        {{-0.5f, -0.5f, -0.5f}, {0.1f, 0.8f, 0.1f, 1.0f}}, // 20
        {{0.5f, 0.5f, -0.5f}, {0.1f, 0.8f, 0.1f, 1.0f}}, // 21
        {{-0.5f, 0.5f, -0.5f}, {0.1f, 0.8f, 0.1f, 1.0f}}, // 22
        {{0.5f, -0.5f, -0.5f}, {0.1f, 0.8f, 0.1f, 1.0f}}, // 23
};

const std::vector<std::uint32_t> cube_indices{0,  1,  2,  0,  3,  1,  4,  5,  6,  4,  7,  5,  8,  9,  10, 8,  11, 9,
                                              12, 13, 14, 12, 15, 13, 16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21};

} // namespace blanks

} // namespace sm::arcane::primitive_graphics