// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include <array>
#include <chrono>
#include <cstdint>

#include <spdlog/logger.h>
#include <vulkan/vulkan_raii.hpp>

#include "cameras/camera.hpp"
#include "primitive_graphics/mesh.hpp"
#include "primitive_graphics/shaders/draw_mesh_pipeline.hpp"
#include "vulkan/device.hpp"
#include "vulkan/swapchain.hpp"
#include "window.hpp"

namespace sm::arcane {

struct frame_info_s {
    std::uint32_t frame_index = 0;
    std::uint32_t image_index = 0;
    std::chrono::steady_clock::time_point started_time{};
};

struct prev_frame_info_s {
    float finished_time = 0.0f;
};

struct frame_context_s {
    vk::raii::Semaphore image_available_semaphore = nullptr;
    vk::raii::Semaphore render_finished_semaphore = nullptr;
    vk::raii::Fence in_flight_fence = nullptr;
};

// структура для передачи в более глубокие уровни вызовов
struct render_resources_s {
    vulkan::Swapchain swapchain;
    // depth image & view
    // hdr image & view
    // other intermediate images and views, buffers, etc.
};

// Temporary pipeline struct. Will remove the struct in the future
struct cube_render_resources_s {
    vk::raii::DescriptorPool descriptor_pool;
    std::vector<vulkan::DeviceMemoryBuffer> global_ubos;
    vk::raii::DescriptorSetLayout global_descriptor_set_layout;
    std::vector<vk::raii::DescriptorSet> global_descriptor_sets;
    primitive_graphics::Mesh cube_mesh;
    primitive_graphics::shaders::DynamicDrawMeshPipeline pipeline;
};

class Renderer {
public:
    Renderer(vulkan::Device &device,
             const vulkan::Swapchain &swapchain,
             std::shared_ptr<spdlog::logger> renderer_logger);

    void begin_frame();
    void end_frame();
    void render();

private:
    std::shared_ptr<spdlog::logger> m_logger;
    vulkan::Device &m_device;
    const vulkan::Swapchain &m_swapchain;


    cameras::Camera m_camera;
    cube_render_resources_s m_resources;

    frame_info_s m_current_frame_info{};
    prev_frame_info_s m_prev_frame_info{};

    std::array<frame_context_s, vulkan::g_max_frames_in_flight> m_frames{};
};

} // namespace sm::arcane