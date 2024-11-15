// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include <array>
#include <memory>
#include <vector>

#include <spdlog/logger.h>
#include <vulkan/vulkan_raii.hpp>

#include "cameras/camera.hpp"
#include "primitive_graphics/mesh.hpp"
#include "render/passes/wireframe.hpp"
#include "scene/scene.hpp"
#include "vulkan/device.hpp"
#include "vulkan/swapchain.hpp"

namespace sm::arcane {

// Temporary pipeline struct. Will remove the struct in the future
struct cube_render_resources_s {
    vk::raii::DescriptorPool global_descriptor_pool;
    std::vector<vulkan::DeviceMemoryBuffer> global_ubos;
    vk::raii::DescriptorSetLayout global_descriptor_set_layout;
    std::vector<vk::raii::DescriptorSet> global_descriptor_sets;
    primitive_graphics::Mesh cube_mesh;
};

struct render_args_s {
    scene::Scene &scene;
};

class Renderer {
public:
    Renderer(vulkan::Device &device,
             const vulkan::Swapchain &swapchain,
             std::shared_ptr<spdlog::logger> renderer_logger);

    void begin_frame();
    void end_frame();
    void render(render_args_s args);

    [[nodiscard]] const render::frame_info_s &frame_info() const noexcept { return m_current_frame_info; }

private:
    std::shared_ptr<spdlog::logger> m_logger;
    vulkan::Device &m_device;
    const vulkan::Swapchain &m_swapchain;

    cube_render_resources_s m_resources;

    render::frame_info_s m_current_frame_info{};
    render::prev_frame_info_s m_prev_frame_info{};

    std::array<render::frame_context_s, vulkan::g_max_frames_in_flight> m_frames{};

    render::passes::Wireframe m_wireframe_pass;
};

} // namespace sm::arcane