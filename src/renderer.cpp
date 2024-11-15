#include "renderer.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <tuple>
#include <utility>

#include "render/common.hpp"

namespace sm::arcane {

namespace {

vk::raii::DescriptorSetLayout make_descriptor_set_layout(
        const vk::raii::Device &device,
        const std::vector<std::tuple<vk::DescriptorType, std::uint32_t, const vk::ShaderStageFlags>> &binding_data,
        vk::DescriptorSetLayoutCreateFlags flags = {}) {
    auto bindings = std::vector<vk::DescriptorSetLayoutBinding>(binding_data.size());
    for (auto i = std::size_t{0}; i < binding_data.size(); ++i) {
        bindings[i] = vk::DescriptorSetLayoutBinding(static_cast<std::uint32_t>(i),
                                                     std::get<0>(binding_data[i]),
                                                     std::get<1>(binding_data[i]),
                                                     std::get<2>(binding_data[i]));
    }
    return {device, {flags, bindings}};
}

void update_descriptor_sets(
        const vk::raii::Device &device,
        const vk::raii::DescriptorSet &descriptor_set,
        const std::vector<
                std::tuple<vk::DescriptorType, const vk::raii::Buffer &, vk::DeviceSize, const vk::raii::BufferView *>>
                &buffer_data,
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

struct global_ubo_s {
    glm::f32mat4 projection{1.0f};
    glm::f32mat4 view{1.0f};
    glm::f32mat4 inverseView{1.0f};
};

[[nodiscard]] inline std::vector<vulkan::DeviceMemoryBuffer> create_global_ubos(
        const vk::raii::PhysicalDevice &physical_device,
        const vk::raii::Device &device) {
    auto global_ubos = std::vector<vulkan::DeviceMemoryBuffer>{};
    global_ubos.reserve(vulkan::g_max_frames_in_flight);
    for (auto i = std::size_t{0}; i < vulkan::g_max_frames_in_flight; ++i) {
        global_ubos.emplace_back(vulkan::DeviceMemoryBuffer{physical_device,
                                                            device,
                                                            vk::BufferUsageFlagBits::eUniformBuffer,
                                                            sizeof(global_ubo_s)});
    }
    return global_ubos;
}

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
                                                              const vk::CommandPool command_pool) {
    auto global_descriptor_pool = make_descriptor_pool(
            device.device(),
            {{vk::DescriptorType::eUniformBuffer, vulkan::g_max_frames_in_flight}});

    auto global_descriptor_set_layout = make_descriptor_set_layout(
            device.device(),
            {{vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex}});

    const auto layouts = std::vector<vk::DescriptorSetLayout>{vulkan::g_max_frames_in_flight,
                                                              *global_descriptor_set_layout};
    const auto global_desc_set_alloc_info = vk::DescriptorSetAllocateInfo{*global_descriptor_pool,
                                                                          static_cast<std::uint32_t>(layouts.size()),
                                                                          layouts.data()};
    auto global_descriptor_sets = device.device().allocateDescriptorSets(global_desc_set_alloc_info);

    auto global_ubos = create_global_ubos(device.physical_device(), device.device());

    auto vertices = primitive_graphics::blanks::cube_vertices;
    auto indices = primitive_graphics::blanks::cube_indices;

    return cube_render_resources_s{
            .global_descriptor_pool = std::move(global_descriptor_pool),
            .global_ubos = std::move(global_ubos),
            .global_descriptor_set_layout = std::move(global_descriptor_set_layout),
            .global_descriptor_sets = std::move(global_descriptor_sets),
            .cube_mesh = primitive_graphics::Mesh{device, command_pool, std::move(vertices), std::move(indices)}};
}

[[nodiscard]] std::array<render::frame_context_s, vulkan::g_max_frames_in_flight> create_frame_contexts(
        const vk::raii::Device &device) noexcept {
    auto frame_contexts = std::array<render::frame_context_s, vulkan::g_max_frames_in_flight>{};
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
      m_resources{create_render_resources(device, m_swapchain.command_pool())},
      m_frames{create_frame_contexts(device.device())},
      m_wireframe_pass{device, m_swapchain, m_current_frame_info, m_resources.global_descriptor_set_layout} {}

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

    m_swapchain.command_buffers()[m_current_frame_info.image_index].begin(vk::CommandBufferBeginInfo{});
}

void Renderer::end_frame() {
    const auto &current_frame = m_frames[m_current_frame_info.frame_index];
    const auto &command_buffer = m_swapchain.command_buffers()[m_current_frame_info.image_index];

    command_buffer.end();

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

void Renderer::render(const render_args_s args) {
    begin_frame();
    const auto &camera = args.scene.camera();

    m_resources.global_ubos[m_current_frame_info.frame_index].upload(
            global_ubo_s{camera.matrices().projection_matrix, camera.matrices().view_matrix});
    update_descriptor_sets(m_device.device(),
                           m_resources.global_descriptor_sets[m_current_frame_info.frame_index],
                           {{vk::DescriptorType::eUniformBuffer,
                             m_resources.global_ubos[m_current_frame_info.frame_index].buffer,
                             VK_WHOLE_SIZE,
                             nullptr}},
                           nullptr);

    const auto &command_buffer = m_swapchain.command_buffers()[m_current_frame_info.image_index];

    {
        m_wireframe_pass.begin(command_buffer);

        command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_wireframe_pass.draw_mesh_pipeline().handle());

        command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                          *m_wireframe_pass.draw_mesh_pipeline().layout(),
                                          0,
                                          {*m_resources.global_descriptor_sets[m_current_frame_info.frame_index]},
                                          nullptr);

        m_resources.cube_mesh.bind(*command_buffer);
        m_resources.cube_mesh.draw(*command_buffer);

        m_wireframe_pass.end(command_buffer);
    }

    end_frame();
}

} // namespace sm::arcane