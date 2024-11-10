#include "renderer.hpp"

#include <algorithm>
#include <numeric>

namespace sm::arcane {

namespace {

vk::raii::DescriptorPool make_descriptor_pool(vk::raii::Device const &device,
                                              const std::vector<vk::DescriptorPoolSize> &pool_sizes) {
    assert(!pool_sizes.empty());
    const auto max_sets = static_cast<std::uint32_t>(std::accumulate(
            pool_sizes.begin(),
            pool_sizes.end(),
            0,
            [](const std::uint32_t sum, const vk::DescriptorPoolSize &dps) { return sum + dps.descriptorCount; }));
    assert(0 < max_sets);
    return {device,
            vk::DescriptorPoolCreateInfo{vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, max_sets, pool_sizes}};
}

[[nodiscard]] cube_render_resources_s create_render_resources(const vulkan::Device &device,
                                                              const vk::CommandPool command_pool,
                                                              cameras::Camera &camera) {
    auto descriptor_pool = make_descriptor_pool(device.device(), {{vk::DescriptorType::eUniformBuffer, 1}});

    auto pipeline = primitive_graphics::shaders::DynamicDrawMeshPipeline{device.device(),
                                                                         device.physical_device(),
                                                                         descriptor_pool,
                                                                         vk::Format::eB8G8R8A8Unorm,
                                                                         vk::Format::eD32Sfloat,
                                                                         camera};

    auto vertices = primitive_graphics::blanks::cube_vertices;
    auto indices = primitive_graphics::blanks::cube_indices;


    camera.update();

    return {.descriptor_pool = std::move(descriptor_pool),
            .cube_mesh = primitive_graphics::Mesh{device, command_pool, std::move(vertices), std::move(indices)},
            .pipeline = std::move(pipeline)};
}

[[nodiscard]] std::array<frame_context_s, vulkan::g_max_frames_in_flight> create_frame_contexts(
        const vk::raii::Device &device) noexcept {
    std::array<frame_context_s, vulkan::g_max_frames_in_flight> frame_contexts;
    for (auto &frame : frame_contexts) {
        frame.image_available_semaphore = vk::raii::Semaphore{device, vk::SemaphoreCreateInfo{}};
        frame.render_finished_semaphore = vk::raii::Semaphore{device, vk::SemaphoreCreateInfo{}};
        frame.in_flight_fence = vk::raii::Fence{device, vk::FenceCreateInfo{}};
    }
    return frame_contexts;
}

} // namespace

Renderer::Renderer(vulkan::Device &device,
                   const vulkan::Swapchain &swapchain,
                   std::shared_ptr<spdlog::logger> renderer_logger)
    : m_logger{std::move(renderer_logger)},
      m_device{device},
      m_swapchain{swapchain},
      m_camera{m_swapchain.aspect_ratio()},
      m_resources{create_render_resources(device, m_swapchain.command_pool(), m_camera)},
      m_frames{create_frame_contexts(device.device())} {
    build_command_buffer();
}

void Renderer::build_command_buffer() {
    const auto extent = m_swapchain.extent();

    const auto &swapchain_depth_image = m_swapchain.depth_image().image;
    const auto &swapchain_depth_image_view = m_swapchain.depth_image().image_view;

    auto clear_values = std::array<vk::ClearValue, 2>{};
    clear_values[0].color = vk::ClearColorValue(std::array{0.2f, 0.2f, 0.2f, 0.2f});
    clear_values[1].depthStencil = vk::ClearDepthStencilValue{1.0f, 0};

    constexpr auto color_subresource_range = vk::ImageSubresourceRange{
            vk::ImageAspectFlagBits::eColor,
            0,
            vk::RemainingMipLevels,
            0,
            vk::RemainingArrayLayers,
    };

    auto depth_subresource_range = vk::ImageSubresourceRange{color_subresource_range};
    depth_subresource_range.aspectMask = vk::ImageAspectFlagBits::eDepth;

    auto i = -1;
    for (const auto &command_buffer : m_swapchain.command_buffers()) {
        ++i;

        const auto &swapchain_color_image = m_swapchain.color_images()[i];
        const auto &swapchain_color_image_view = m_swapchain.color_image_views()[i];

        command_buffer.begin(vk::CommandBufferBeginInfo{});

        vulkan::image_layout_transition(*command_buffer,
                                        swapchain_color_image,
                                        vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                        vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                        vk::AccessFlagBits::eNone,
                                        vk::AccessFlagBits::eColorAttachmentWrite,
                                        vk::ImageLayout::eUndefined,
                                        vk::ImageLayout::eColorAttachmentOptimal,
                                        color_subresource_range);

        vulkan::image_layout_transition(*command_buffer,
                                        *swapchain_depth_image,
                                        vk::ImageLayout::eUndefined,
                                        vk::ImageLayout::eDepthAttachmentOptimal,
                                        depth_subresource_range);

        const auto color_attachment = vk::RenderingAttachmentInfoKHR{swapchain_color_image_view,
                                                                     vk::ImageLayout::eColorAttachmentOptimal,
                                                                     vk::ResolveModeFlagBits::eNone,
                                                                     {},
                                                                     {},
                                                                     vk::AttachmentLoadOp::eClear,
                                                                     vk::AttachmentStoreOp::eStore,
                                                                     clear_values[0]};

        const auto depth_attachment = vk::RenderingAttachmentInfoKHR{swapchain_depth_image_view,
                                                                     vk::ImageLayout::eDepthAttachmentOptimal,
                                                                     vk::ResolveModeFlagBits::eNone,
                                                                     {},
                                                                     {},
                                                                     vk::AttachmentLoadOp::eClear,
                                                                     vk::AttachmentStoreOp::eDontCare,
                                                                     clear_values[1]};

        const auto rendering_info = vk::RenderingInfoKHR{
                {},
                vk::Rect2D{{0, 0}, extent},
                1,
                0,
                1,
                &color_attachment,
                &depth_attachment,
                nullptr // TODO: is_depth_only_format() pStencilAttachment
        };

        command_buffer.beginRendering(rendering_info);

        const auto viewport = vk::Viewport{0.0f,
                                           0.0f,
                                           static_cast<float>(extent.width),
                                           static_cast<float>(extent.height),
                                           0.0f,
                                           1.0f};
        command_buffer.setViewport(0, viewport);

        const auto scissor = vk::Rect2D{{0, 0}, extent};
        command_buffer.setScissor(0, scissor);

        command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_resources.pipeline.handle());

        command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                          *m_resources.pipeline.layout(),
                                          0,
                                          {*m_resources.pipeline.desc_set()},
                                          nullptr);

        m_resources.cube_mesh.bind(*command_buffer);
        m_resources.cube_mesh.draw(*command_buffer);

        command_buffer.endRendering();

        vulkan::image_layout_transition(*command_buffer,
                                        swapchain_color_image,
                                        vk::ImageLayout::eColorAttachmentOptimal,
                                        vk::ImageLayout::ePresentSrcKHR,
                                        color_subresource_range);

        command_buffer.end();
    }
}

void Renderer::begin_frame() {
    auto &current_frame = m_frames[m_current_frame_info.frame_index];

    auto [result, image_index] = m_swapchain.get().acquireNextImage(std::numeric_limits<uint64_t>::max(),
                                                                    *current_frame.image_available_semaphore,
                                                                    nullptr);
    assert(image_index < m_swapchain.color_images().size());
    if (result == vk::Result::eErrorOutOfDateKHR) {
        return;
    }
    if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
        throw std::runtime_error("Failed to acquire swapchain image!");
    }

    m_current_frame_info.image_index = image_index;

    m_current_frame_info.started_time = std::chrono::steady_clock::now();
}

void Renderer::end_frame() {
    const auto &current_frame = m_frames[m_current_frame_info.frame_index];
    const auto &command_buffer = m_swapchain.command_buffers()[m_current_frame_info.image_index];

    constexpr auto wait_stages = vk::PipelineStageFlags{vk::PipelineStageFlagBits::eColorAttachmentOutput};

    auto submit_info = vk::SubmitInfo{
            *current_frame.image_available_semaphore, // Wait semaphore
            wait_stages, // Wait stages
            *command_buffer, // Command buffer
            *current_frame.render_finished_semaphore // Signal semaphore
    };

    m_device.queue_families().graphics.queue.submit(submit_info,
                                                    *m_frames[m_current_frame_info.frame_index].in_flight_fence);

    while (vk::Result::eTimeout ==
           m_device.device().waitForFences({*m_frames[m_current_frame_info.frame_index].in_flight_fence},
                                           VK_TRUE,
                                           std::numeric_limits<std::uint64_t>::max()))
        ;
    m_device.device().resetFences(*m_frames[m_current_frame_info.frame_index].in_flight_fence);

    auto present_info = vk::PresentInfoKHR{1,
                                           &*current_frame.render_finished_semaphore,
                                           1,
                                           &*m_swapchain.get(),
                                           &m_current_frame_info.image_index};

    const auto result = m_device.queue_families().present.queue.presentKHR(present_info);

    if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR) {
    } else if (result != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to present swapchain image!");
    }

    auto current_frame_finished_time = std::chrono::steady_clock::now();
    m_prev_frame_info.finished_time =
            std::chrono::duration<float>(current_frame_finished_time - m_current_frame_info.started_time).count();

    m_current_frame_info.frame_index = (m_current_frame_info.frame_index + 1) % vulkan::g_max_frames_in_flight;
}

void Renderer::render() {
    begin_frame();
    end_frame();
}

} // namespace sm::arcane