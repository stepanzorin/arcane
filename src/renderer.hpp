// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024-2025 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include <array>
#include <memory>
#include <vector>

#include <spdlog/logger.h>
#include <vulkan/vulkan_raii.hpp>

#include "frame.hpp"
#include "primitive_graphics/mesh.hpp"
#include "render/passes/gbuffer.hpp"
#include "scene/scene.hpp"
#include "vulkan/device.hpp"
#include "vulkan/swapchain.hpp"

namespace sm::arcane {

inline static constexpr auto g_max_frames_in_flight = 2u;

// Temporary pipeline struct. Will remove the struct in the future
struct global_resources_s {
    vk::raii::DescriptorPool global_descriptor_pool;
    std::vector<vulkan::DeviceMemoryBuffer> global_ubos;
    vk::raii::DescriptorSetLayout global_descriptor_set_layout;
    std::vector<vk::raii::DescriptorSet> global_descriptor_sets;
};

struct render_context_s {
    scene::Scene &scene;
};

class Renderer {
public:
    Renderer(vulkan::Device &device,
             std::unique_ptr<vulkan::Swapchain> &swapchain,
             std::shared_ptr<spdlog::logger> renderer_logger);

    void begin_frame();
    void end_frame();
    void render(render_context_s args);

    [[nodiscard]] const frame_info_s &frame_info() const noexcept { return m_current_frame_info; }

private:
    std::shared_ptr<spdlog::logger> m_logger;
    vulkan::Device &m_device;
    const std::unique_ptr<vulkan::Swapchain> &m_swapchain;

    global_resources_s m_resources;

    frame_info_s &m_current_frame_info;
    prev_frame_info_s m_prev_frame_info{};

    struct frame_sync_s {
        struct semaphores_s {
            vk::raii::Semaphore image_available = nullptr;
            vk::raii::Semaphore render_finished = nullptr;
        } semaphores;

        struct fences_s {
            vk::raii::Fence in_flight = nullptr;
        } fences;
    };
    std::array<frame_sync_s, g_max_frames_in_flight> m_frame_syncs{};

    render::passes::Gbuffer m_gbuffer;
};

} // namespace sm::arcane