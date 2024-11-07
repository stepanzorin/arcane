// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

// TODO: This is a temporary solution! Move in a separate place in the future

#pragma once

#include <filesystem>
#include <utility>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

namespace sm::arcane::common::shaders {

namespace detail {

struct shader_module_s final {
    vk::UniqueShaderModule vertex;
    vk::UniqueShaderModule fragment;
};

} // namespace detail

using shader_data_t = std::vector<char>;
using vertex_data_t = shader_data_t;
using fragment_data_t = shader_data_t;

[[nodiscard]] std::pair<vertex_data_t, fragment_data_t> read_spirv_files(const std::filesystem::path &file_name);

[[nodiscard]] vk::raii::ShaderModule create_shader_module(const vk::raii::Device &device,
                                                          const std::vector<char> &code) noexcept;

} // namespace sm::arcane::common::shaders