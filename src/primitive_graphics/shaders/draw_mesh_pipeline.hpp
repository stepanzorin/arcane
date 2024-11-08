#pragma once

#include <cstddef>
#include <cstdint>
#include <tuple>
#include <vector>

#include <glm/fwd.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <vulkan/vulkan_raii.hpp>

#include "common/shaders/pipeline_functions.hpp"
#include "vulkan/device.hpp"
#include "vulkan/device_memory.hpp"
#include "vulkan/image_barriers.hpp"

namespace sm::arcane::primitive_graphics::shaders {

struct Vertex {
    float x, y, z, w; // Position
    float r, g, b, a; // Color
};

inline glm::mat4x4 create_model_view_projection_clip_matrix(vk::Extent2D const &extent) {
    auto fov = glm::radians(30.0f);
    if (extent.width > extent.height) {
        fov *= static_cast<float>(extent.height) / static_cast<float>(extent.width);
    }

    const auto model = glm::mat4x4(1.0f);
    const auto view = glm::lookAt(glm::f32vec3{-5.0f, 3.0f, -10.0f},
                                  glm::f32vec3{0.0f, 0.0f, 0.0f},
                                  glm::f32vec3{0.0f, -1.0f, 0.0f});
    const auto projection = glm::perspective(fov, 1.0f, 0.1f, 100.0f);
    // clang-format off
    const auto clip = glm::f32mat4x4{ 1.0f,  0.0f, 0.0f, 0.0f,
                                    0.0f, -1.0f, 0.0f, 0.0f,
                                    0.0f,  0.0f, 0.5f, 0.0f,
                                    0.0f,  0.0f, 0.5f, 1.0f };  // vulkan clip space has inverted y and half z !
    // clang-format on
    return clip * projection * view * model;
}

inline vk::raii::DescriptorSetLayout make_descriptor_set_layout(
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

inline void update_descriptor_sets(
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

class DynamicDrawMeshPipeline {
public:
    DynamicDrawMeshPipeline(const vk::raii::Device &device,
                            const vk::raii::PhysicalDevice &physical_device,
                            const vk::raii::DescriptorPool &descriptor_pool,
                            const vk::Format color_format,
                            const vk::Format depth_format,
                            const vk::Extent2D &surface_extent = {1600, 900})
        : m_device(device),
          m_uniform_buffer{vulkan::DeviceMemoryBuffer{physical_device,
                                                      device,
                                                      vk::BufferUsageFlagBits::eUniformBuffer,
                                                      sizeof(glm::f32mat4)}},
          m_descriptor_set_layout{make_descriptor_set_layout(
                  device,
                  {{vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex}})},
          m_descriptor_set{
                  std::move(vk::raii::DescriptorSets(device, {descriptor_pool, *m_descriptor_set_layout}).front())},
          m_pipeline_layout(vk::raii::PipelineLayout(m_device, {{}, *m_descriptor_set_layout})),
          m_pipeline_cache{m_device, vk::PipelineCacheCreateInfo{}},
          m_pipeline(nullptr),
          m_color_format(color_format),
          m_depth_format(depth_format) {
        m_uniform_buffer.upload(create_model_view_projection_clip_matrix(surface_extent));
        update_descriptor_sets(device,
                               m_descriptor_set,
                               {{vk::DescriptorType::eUniformBuffer, m_uniform_buffer.buffer, VK_WHOLE_SIZE, nullptr}},
                               nullptr);


        createPipeline(m_device);
    }

    ~DynamicDrawMeshPipeline() = default;

    [[nodiscard]] const vk::raii::Pipeline &handle() { return m_pipeline; }
    [[nodiscard]] const vk::raii::PipelineLayout &layout() { return m_pipeline_layout; }
    [[nodiscard]] const vk::raii::DescriptorSet &desc_set() { return m_descriptor_set; }

    DynamicDrawMeshPipeline(DynamicDrawMeshPipeline &&other) noexcept = default;

private:
    void createPipeline(const vk::raii::Device &device) {
        auto [vertex_code, fragment_code] = common::shaders::read_spirv_files("draw_mesh");
        auto vertex_shader_module = common::shaders::create_shader_module(device, vertex_code);
        auto fragment_shader_module = common::shaders::create_shader_module(device, fragment_code);

        constexpr vk::PipelineInputAssemblyStateCreateInfo input_assembly_state({},
                                                                                vk::PrimitiveTopology::eTriangleList);
        constexpr vk::PipelineRasterizationStateCreateInfo rasterization_state({},
                                                                               false,
                                                                               false,
                                                                               vk::PolygonMode::eFill,
                                                                               vk::CullModeFlagBits::eBack,
                                                                               vk::FrontFace::eCounterClockwise,
                                                                               false,
                                                                               0.0f,
                                                                               0.0f,
                                                                               0.0f,
                                                                               1.0f);

        const auto colorComponentFlags = vk::ColorComponentFlags{
                vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
                vk::ColorComponentFlagBits::eA};

        const auto color_attachment_state = vk::PipelineColorBlendAttachmentState{false,
                                                                                  vk::BlendFactor::eZero,
                                                                                  vk::BlendFactor::eZero,
                                                                                  vk::BlendOp::eAdd,
                                                                                  vk::BlendFactor::eZero,
                                                                                  vk::BlendFactor::eZero,
                                                                                  vk::BlendOp::eAdd,
                                                                                  colorComponentFlags};

        const auto color_blend_state = vk::PipelineColorBlendStateCreateInfo{{},
                                                                             false,
                                                                             vk::LogicOp::eNoOp,
                                                                             color_attachment_state,
                                                                             {{1.0f, 1.0f, 1.0f, 1.0f}}};

        constexpr auto stencil_op_state = vk::StencilOpState{vk::StencilOp::eKeep,
                                                             vk::StencilOp::eKeep,
                                                             vk::StencilOp::eKeep,
                                                             vk::CompareOp::eAlways};
        constexpr auto depth_stencil_state = vk::PipelineDepthStencilStateCreateInfo{{},
                                                                                     true,
                                                                                     true,
                                                                                     vk::CompareOp::eLessOrEqual,
                                                                                     false,
                                                                                     false,
                                                                                     stencil_op_state,
                                                                                     stencil_op_state};

        constexpr auto dynamic_state_enables = std::array{vk::DynamicState::eViewport, vk::DynamicState::eScissor};
        const auto dynamic_state_info = vk::PipelineDynamicStateCreateInfo{
                {},
                static_cast<std::uint32_t>(dynamic_state_enables.size()),
                dynamic_state_enables.data()};

        constexpr auto vertex_input_bindings = std::array{
                vk::VertexInputBindingDescription{0, sizeof(Vertex), vk::VertexInputRate::eVertex}};

        constexpr auto vertex_input_attributes = std::array{
                vk::VertexInputAttributeDescription{0, 0, vk::Format::eR32G32B32A32Sfloat, 0},
                vk::VertexInputAttributeDescription{1, 0, vk::Format::eR32G32B32A32Sfloat, 16}};

        const auto vertex_input_info = vk::PipelineVertexInputStateCreateInfo{
                {},
                static_cast<std::uint32_t>(vertex_input_bindings.size()),
                vertex_input_bindings.data(),
                static_cast<std::uint32_t>(vertex_input_attributes.size()),
                vertex_input_attributes.data()};

        const auto shader_stages = std::array{
                vk::PipelineShaderStageCreateInfo{{}, vk::ShaderStageFlagBits::eVertex, vertex_shader_module, "main"},
                vk::PipelineShaderStageCreateInfo{{},
                                                  vk::ShaderStageFlagBits::eFragment,
                                                  fragment_shader_module,
                                                  "main"}};

        constexpr auto multisampling = vk::PipelineMultisampleStateCreateInfo{{}, vk::SampleCountFlagBits::e1};
        constexpr auto viewport_state = vk::PipelineViewportStateCreateInfo{{}, 1, nullptr, 1, nullptr};

        const auto rendering_create_info_khr = vk::PipelineRenderingCreateInfoKHR{
                {},
                1,
                &m_color_format,
                m_depth_format,
                /* TODO:
                * if (!vkb::is_depth_only_format(depth_format))
                      {
                          pipeline_create.stencilAttachmentFormat =
                depth_format;
                      }
                 */
                {}};

        const auto pipeline_info = vk::GraphicsPipelineCreateInfo{{},
                                                                  shader_stages,
                                                                  &vertex_input_info,
                                                                  &input_assembly_state,
                                                                  nullptr,
                                                                  &viewport_state,
                                                                  &rasterization_state,
                                                                  &multisampling,
                                                                  &depth_stencil_state,
                                                                  &color_blend_state,
                                                                  &dynamic_state_info,
                                                                  *m_pipeline_layout,
                                                                  {},
                                                                  {},
                                                                  {},
                                                                  {},
                                                                  &rendering_create_info_khr};

        m_pipeline = vk::raii::Pipeline(m_device, m_pipeline_cache, pipeline_info);
    }

    const vk::raii::Device &m_device;

    vulkan::DeviceMemoryBuffer m_uniform_buffer;
    vk::raii::DescriptorSetLayout m_descriptor_set_layout;
    vk::raii::DescriptorSet m_descriptor_set;

    vk::raii::PipelineLayout m_pipeline_layout;
    vk::raii::PipelineCache m_pipeline_cache;
    vk::raii::Pipeline m_pipeline;

    vk::Format m_color_format;
    vk::Format m_depth_format;
};

} // namespace sm::arcane::primitive_graphics::shaders
