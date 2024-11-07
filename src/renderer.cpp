#include "renderer.hpp"

#include <numeric>

namespace sm::arcane {

namespace {

struct vertex_s {
    float x, y, z, w; // Position
    float r, g, b, a; // Color
};

void update_descriptor_sets(vk::raii::Device const &device,
                            vk::raii::DescriptorSet const &descriptor_set,
                            std::vector<std::tuple<vk::DescriptorType,
                                                   vk::raii::Buffer const &,
                                                   vk::DeviceSize,
                                                   vk::raii::BufferView const *>> const &buffer_data,
                            std::nullptr_t,
                            const std::uint32_t binding_offset = 0) {
    std::vector<vk::DescriptorBufferInfo> bufferInfos;
    bufferInfos.reserve(buffer_data.size());

    std::vector<vk::WriteDescriptorSet> writeDescriptorSets;
    writeDescriptorSets.reserve(buffer_data.size());
    std::uint32_t dstBinding = binding_offset;
    for (auto const &bd : buffer_data) {
        bufferInfos.emplace_back(std::get<1>(bd), 0, std::get<2>(bd));
        vk::BufferView bufferView;
        if (std::get<3>(bd)) {
            bufferView = *std::get<3>(bd);
        }
        writeDescriptorSets.emplace_back(descriptor_set,
                                         dstBinding++,
                                         0,
                                         1,
                                         std::get<0>(bd),
                                         nullptr,
                                         &bufferInfos.back(),
                                         std::get<3>(bd) ? &bufferView : nullptr);
    }

    device.updateDescriptorSets(writeDescriptorSets, nullptr);
}

const vertex_s colored_cube_data[] = {
        // red face
        {-1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f},
        {-1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f},
        {1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f},
        {1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f},
        {-1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f},
        // green face
        {-1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f},
        {1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f},
        {-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f},
        {-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f},
        {1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f},
        {1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f},
        // blue face
        {-1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f},
        {-1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f},
        {-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f},
        {-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f},
        {-1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f},
        {-1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f},
        // yellow face
        {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f},
        {1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f},
        {1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f},
        {1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f},
        {1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f},
        {1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f},
        // magenta face
        {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f},
        {-1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f},
        {-1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f},
        {-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f},
        // cyan face
        {1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f},
        {1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f},
        {-1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f},
        {-1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f},
        {1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f},
        {-1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f},
};

vk::raii::DescriptorPool makeDescriptorPool(vk::raii::Device const &device,
                                            std::vector<vk::DescriptorPoolSize> const &poolSizes) {
    assert(!poolSizes.empty());
    uint32_t maxSets = std::accumulate(
            poolSizes.begin(),
            poolSizes.end(),
            0,
            [](uint32_t sum, vk::DescriptorPoolSize const &dps) { return sum + dps.descriptorCount; });
    assert(0 < maxSets);

    vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
                                                          maxSets,
                                                          poolSizes);
    return vk::raii::DescriptorPool(device, descriptorPoolCreateInfo);
}

[[nodiscard]] bool is_depth_only_format(const vk::Format format) noexcept {
    return format == vk::Format::eD16Unorm || format == vk::Format::eD32Sfloat;
}

/*bool is_depth_stencil_format(VkFormat format) {
    return format == VK_FORMAT_D16_UNORM_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT ||
           format == VK_FORMAT_D32_SFLOAT_S8_UINT;
}

bool is_depth_format(VkFormat format) { return is_depth_only_format(format) || is_depth_stencil_format(format); }*/

[[nodiscard]] cube_render_resources_s create_render_resources(const vulkan::Device &device) {
    auto descriptor_pool = makeDescriptorPool(device.device(), {{vk::DescriptorType::eUniformBuffer, 1}});

    auto vertex_buffer = device.create_device_memory_buffer(vk::BufferUsageFlagBits::eVertexBuffer,
                                                            sizeof(colored_cube_data));

    vertex_buffer.upload(colored_cube_data, sizeof(colored_cube_data) / sizeof(colored_cube_data[0]));

    auto pipeline = primitive_graphics::shaders::DynamicDrawMeshPipeline{device.device(),
                                                                         device.physical_device(),
                                                                         descriptor_pool,
                                                                         vk::Format::eB8G8R8A8Unorm,
                                                                         vk::Format::eD32Sfloat};

    return {.descriptor_pool = std::move(descriptor_pool),
            .vertex_buffer = std::move(vertex_buffer),
            .pipeline = std::move(pipeline)};
}

} // namespace

Renderer::Renderer(vulkan::Device &device,
                   const vulkan::Swapchain &swapchain,
                   std::shared_ptr<spdlog::logger> renderer_logger)
    : m_logger{std::move(renderer_logger)},
      m_device{device},
      m_swapchain{swapchain},
      m_resources{create_render_resources(device)} {
    for (auto &frame : m_frames) {
        frame.image_available_semaphore = vk::raii::Semaphore{m_device.device(), vk::SemaphoreCreateInfo{}};
        frame.render_finished_semaphore = vk::raii::Semaphore{m_device.device(), vk::SemaphoreCreateInfo{}};
        frame.in_flight_fence = vk::raii::Fence{m_device.device(), vk::FenceCreateInfo{}};
    }

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
            /*.aspectMask = */ vk::ImageAspectFlagBits::eColor,
            /*.baseMipLevel = */ 0,
            /*.levelCount = */ vk::RemainingMipLevels,
            /*.baseArrayLayer = */ 0,
            /*.layerCount = */ vk::RemainingArrayLayers,
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

        vk::RenderingAttachmentInfoKHR color_attachment{swapchain_color_image_view,
                                                        vk::ImageLayout::eColorAttachmentOptimal,
                                                        vk::ResolveModeFlagBits::eNone,
                                                        {},
                                                        {},
                                                        vk::AttachmentLoadOp::eClear,
                                                        vk::AttachmentStoreOp::eStore,
                                                        clear_values[0]};

        vk::RenderingAttachmentInfoKHR depth_attachment{swapchain_depth_image_view,
                                                        vk::ImageLayout::eDepthAttachmentOptimal,
                                                        vk::ResolveModeFlagBits::eNone,
                                                        {},
                                                        {},
                                                        vk::AttachmentLoadOp::eClear,
                                                        vk::AttachmentStoreOp::eDontCare,
                                                        clear_values[1]};

        vk::RenderingInfoKHR rendering_info{
                {},
                vk::Rect2D{{0, 0}, extent},
                1, // layerCount
                0, // viewMask
                1, // colorAttachmentCount
                &color_attachment, // pColorAttachments
                &depth_attachment, // pDepthAttachment
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

        command_buffer.bindVertexBuffers(0, *m_resources.vertex_buffer.buffer, {0});
        command_buffer.draw(36, 1, 0, 0);

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