// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include "objects/systems.hpp"
#include "render/common.hpp"
#include "render/passes/common.hpp"
#include "vulkan/image_barriers.hpp"
#include "vulkan/swapchain.hpp"

#include <spdlog/spdlog.h>

namespace sm::arcane::render::passes {

class Gbuffer {
public:
    Gbuffer(const pass_context_s &pass_context)
        : m_swapchain{pass_context.swapchain},
          m_draw_game_object_system{pass_context} {}

    void render(const render_args_s &args, gpu_resources_s &global_gpu_resources) const {
        begin(args.command_buffer, global_gpu_resources);
        {
            m_draw_game_object_system.render(args); //
        }
        end(args.command_buffer, global_gpu_resources);
    }

private:
    void begin(const vk::raii::CommandBuffer &command_buffer, gpu_resources_s &global_gpu_resources) const {
        constexpr auto clear_values = std::array<vk::ClearValue, 2>{
                vk::ClearColorValue{std::array{0.2f, 0.2f, 0.2f, 0.2f}},
                vk::ClearDepthStencilValue{1.0f, 0}};

        vulkan::image_layout_transition(*command_buffer,
                                        *global_gpu_resources.gbuffer.albedo.handle.image,
                                        vk::ImageLayout::eUndefined,
                                        vk::ImageLayout::eColorAttachmentOptimal,
                                        g_color_subresource_range);

        vulkan::image_layout_transition(*command_buffer,
                                        *global_gpu_resources.depth_stencil.handle.image,
                                        vk::ImageLayout::eUndefined,
                                        vk::ImageLayout::eDepthAttachmentOptimal,
                                        g_depth_subresource_range);

        const auto color_attachment = vk::RenderingAttachmentInfoKHR{
                *global_gpu_resources.gbuffer.albedo.handle.image_view,
                vk::ImageLayout::eColorAttachmentOptimal,
                vk::ResolveModeFlagBits::eNone,
                {},
                {},
                vk::AttachmentLoadOp::eLoad,
                vk::AttachmentStoreOp::eStore,
                clear_values[0]};

        const auto depth_attachment = vk::RenderingAttachmentInfoKHR{
                *global_gpu_resources.depth_stencil.handle.image_view,
                vk::ImageLayout::eDepthAttachmentOptimal,
                vk::ResolveModeFlagBits::eNone,
                {},
                {},
                vk::AttachmentLoadOp::eClear,
                vk::AttachmentStoreOp::eDontCare,
                clear_values[1]};

        const auto rendering_info = vk::RenderingInfoKHR{{},
                                                         vk::Rect2D{{0, 0}, m_swapchain->extent()},
                                                         1,
                                                         0,
                                                         1,
                                                         &color_attachment,
                                                         &depth_attachment,
                                                         nullptr};
        command_buffer.beginRendering(rendering_info);

        const auto scissor = vk::Rect2D{{0, 0}, m_swapchain->extent()};
        command_buffer.setScissor(0, scissor);

        const auto viewport = vk::Viewport{0.0f,
                                           0.0f,
                                           static_cast<float>(m_swapchain->extent().width),
                                           static_cast<float>(m_swapchain->extent().height),
                                           0.0f,
                                           1.0f};
        command_buffer.setViewport(0, viewport);
    }

    void end(const vk::raii::CommandBuffer &command_buffer, gpu_resources_s &global_gpu_resources) const {
        command_buffer.endRendering();
    }

    const std::unique_ptr<vulkan::Swapchain> &m_swapchain;

    objects::DrawGameObjectSystem m_draw_game_object_system;
};

} // namespace sm::arcane::render::passes