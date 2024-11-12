// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include "primitive_graphics/shaders/draw_mesh_pipeline.hpp"
#include "render/common.hpp"
#include "render/passes/common.hpp"
#include "vulkan/swapchain.hpp"

#include <spdlog/spdlog.h>

namespace sm::arcane::render::passes {

// Wireframe pass - contains only vertex and color data
class Wireframe {
public:
    Wireframe(const vulkan::Device &device,
              const vulkan::Swapchain &swapchain,
              const frame_info_s &frame_info,
              const vk::DescriptorSetLayout global_desc_set_layout)
        : m_swapchain{swapchain},
          m_frame_info{frame_info},
          m_draw_mesh_pipeline{device.device(),
                               global_desc_set_layout,
                               m_swapchain.color_format(),
                               m_swapchain.depth_format()} {}

    void begin(const vk::raii::CommandBuffer &command_buffer) const {
        constexpr auto clear_values = std::array<vk::ClearValue, 2>{
                vk::ClearColorValue{std::array{0.2f, 0.2f, 0.2f, 0.2f}},
                vk::ClearDepthStencilValue{1.0f, 0}};

        vulkan::image_layout_transition(*command_buffer,
                                        m_swapchain.color_images()[m_frame_info.image_index],
                                        vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                        vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                        vk::AccessFlagBits::eNone,
                                        vk::AccessFlagBits::eColorAttachmentWrite,
                                        vk::ImageLayout::eUndefined,
                                        vk::ImageLayout::eColorAttachmentOptimal,
                                        color_subresource_range);

        vulkan::image_layout_transition(*command_buffer,
                                        *m_swapchain.depth_image().image,
                                        vk::ImageLayout::eUndefined,
                                        vk::ImageLayout::eDepthAttachmentOptimal,
                                        depth_subresource_range);

        const auto color_attachment = vk::RenderingAttachmentInfoKHR{
                m_swapchain.color_image_views()[m_frame_info.image_index],
                vk::ImageLayout::eColorAttachmentOptimal,
                vk::ResolveModeFlagBits::eNone,
                {},
                {},
                vk::AttachmentLoadOp::eClear,
                vk::AttachmentStoreOp::eStore,
                clear_values[0]};

        const auto depth_attachment = vk::RenderingAttachmentInfoKHR{m_swapchain.depth_image().image_view,
                                                                     vk::ImageLayout::eDepthAttachmentOptimal,
                                                                     vk::ResolveModeFlagBits::eNone,
                                                                     {},
                                                                     {},
                                                                     vk::AttachmentLoadOp::eClear,
                                                                     vk::AttachmentStoreOp::eDontCare,
                                                                     clear_values[1]};

        const auto rendering_info = vk::RenderingInfoKHR{
                {},
                vk::Rect2D{{0, 0}, m_swapchain.extent()},
                1,
                0,
                1,
                &color_attachment,
                &depth_attachment,
                nullptr // TODO: is_depth_only_format() pStencilAttachment
        };

        command_buffer.beginRendering(rendering_info);

        const auto scissor = vk::Rect2D{{0, 0}, m_swapchain.extent()};
        command_buffer.setScissor(0, scissor);

        const auto viewport = vk::Viewport{0.0f,
                                           0.0f,
                                           static_cast<float>(m_swapchain.extent().width),
                                           static_cast<float>(m_swapchain.extent().height),
                                           0.0f,
                                           1.0f};
        command_buffer.setViewport(0, viewport);
    }

    void end(const vk::raii::CommandBuffer &command_buffer) const {
        command_buffer.endRendering();

        vulkan::image_layout_transition(*command_buffer,
                                        m_swapchain.color_images()[m_frame_info.image_index],
                                        vk::ImageLayout::eColorAttachmentOptimal,
                                        vk::ImageLayout::ePresentSrcKHR,
                                        color_subresource_range);
    }

    [[nodiscard]] const primitive_graphics::shaders::DynamicDrawMeshPipeline &draw_mesh_pipeline() const noexcept {
        return m_draw_mesh_pipeline;
    }

private:
    const vulkan::Swapchain &m_swapchain;
    const frame_info_s &m_frame_info;

    primitive_graphics::shaders::DynamicDrawMeshPipeline m_draw_mesh_pipeline;
};

} // namespace sm::arcane::render::passes