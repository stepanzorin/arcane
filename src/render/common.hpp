// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include "vulkan/swapchain.hpp"

namespace sm::arcane::render {

struct global_render_args {
    vk::DescriptorSetLayout descriptor_set_layout;
    vk::DescriptorSet descriptor_set;
};

struct render_args {
    vulkan::Device &device;
    const std::unique_ptr<vulkan::Swapchain> &swapchain;
    const vk::raii::CommandBuffer &command_buffer;
    global_render_args global;
};

struct create_resource_args_s {
    const vulkan::Device &device;
    const std::unique_ptr<vulkan::Swapchain> &swapchain;
    global_render_args global;
};

// структура для передачи в более глубокие уровни вызовов
struct render_resources_s {
    // vulkan::Swapchain swapchain;
    // depth image & view
    // hdr image & view
    // other intermediate images and views, buffers, etc.
};

} // namespace sm::arcane::render