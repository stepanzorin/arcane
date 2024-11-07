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
    float fov = glm::radians(30.0f);
    if (extent.width > extent.height) {
        fov *= static_cast<float>(extent.height) / static_cast<float>(extent.width);
    }

    glm::mat4x4 model = glm::mat4x4(1.0f);
    glm::mat4x4 view = glm::lookAt(glm::f32vec3(-5.0f, 3.0f, -10.0f),
                                   glm::f32vec3(0.0f, 0.0f, 0.0f),
                                   glm::f32vec3(0.0f, -1.0f, 0.0f));
    glm::mat4x4 projection = glm::perspective(fov, 1.0f, 0.1f, 100.0f);
    // clang-format off
    glm::mat4x4 clip = glm::f32mat4x4( 1.0f,  0.0f, 0.0f, 0.0f,
                                    0.0f, -1.0f, 0.0f, 0.0f,
                                    0.0f,  0.0f, 0.5f, 0.0f,
                                    0.0f,  0.0f, 0.5f, 1.0f );  // vulkan clip space has inverted y and half z !
    // clang-format on
    return clip * projection * view * model;
}

inline vk::raii::DescriptorSetLayout make_descriptor_set_layout(
        vk::raii::Device const &device,
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

inline void update_descriptor_sets(vk::raii::Device const &device,
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

inline vk::DescriptorSetAllocateInfo descriptor_set_allocate_info(const vk::DescriptorPool descriptor_pool,
                                                                  const vk::DescriptorSetLayout set_layouts,
                                                                  const std::uint32_t descriptor_set_count) {
    vk::DescriptorSetAllocateInfo descriptor_set_allocate_info{};
    descriptor_set_allocate_info.setDescriptorPool(descriptor_pool);
    descriptor_set_allocate_info.setSetLayouts(set_layouts);
    descriptor_set_allocate_info.setDescriptorSetCount(descriptor_set_count);

    return descriptor_set_allocate_info;
}

inline vk::DescriptorImageInfo descriptor_image_info(const vk::raii::Sampler &sampler,
                                                     const vk::raii::ImageView &image_view,
                                                     vk::ImageLayout image_layout) {
    vk::DescriptorImageInfo descriptor_image_info{};
    descriptor_image_info.setSampler(*sampler);
    descriptor_image_info.setImageView(*image_view);
    descriptor_image_info.setImageLayout(image_layout);

    return descriptor_image_info;
}

inline vk::WriteDescriptorSet write_descriptor_set(const vk::raii::DescriptorSet &dst_set,
                                                   vk::DescriptorType type,
                                                   uint32_t binding,
                                                   vk::DescriptorBufferInfo *buffer_info,
                                                   uint32_t descriptor_count = 1) {
    vk::WriteDescriptorSet write_descriptor_set{};
    write_descriptor_set.setDstSet(*dst_set);
    write_descriptor_set.setDescriptorType(type);
    write_descriptor_set.setDstBinding(binding);
    write_descriptor_set.setBufferInfo(*buffer_info);
    write_descriptor_set.setDescriptorCount(descriptor_count);

    return write_descriptor_set;
}

inline vk::WriteDescriptorSet write_descriptor_set(const vk::raii::DescriptorSet &dst_set,
                                                   vk::DescriptorType type,
                                                   uint32_t binding,
                                                   vk::DescriptorImageInfo *image_info,
                                                   uint32_t descriptor_count = 1) {
    vk::WriteDescriptorSet write_descriptor_set{};
    write_descriptor_set.setDstSet(*dst_set);
    write_descriptor_set.setDescriptorType(type);
    write_descriptor_set.setDstBinding(binding);
    write_descriptor_set.setImageInfo(*image_info);
    write_descriptor_set.setDescriptorCount(descriptor_count);

    return write_descriptor_set;
}

inline vk::DescriptorBufferInfo create_descriptor(const vk::Buffer buffer,
                                                  const vk::DeviceSize size = vk::WholeSize,
                                                  const vk::DeviceSize offset = 0) noexcept {
    return {buffer, offset, size};
}


inline vk::DescriptorSetLayoutBinding descriptor_set_layout_binding(const vk::DescriptorType type,
                                                                    const vk::ShaderStageFlags flags,
                                                                    const std::uint32_t binding,
                                                                    const std::uint32_t count = 1) noexcept {
    return {binding, type, count, flags, nullptr};
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
          m_pipeline_layout(nullptr),
          m_pipeline_cache{m_device, vk::PipelineCacheCreateInfo{}},
          m_pipeline(nullptr),
          m_color_format(color_format),
          m_depth_format(depth_format) {
        m_uniform_buffer.upload(create_model_view_projection_clip_matrix(surface_extent));
        update_descriptor_sets(device,
                               m_descriptor_set,
                               {{vk::DescriptorType::eUniformBuffer, m_uniform_buffer.buffer, VK_WHOLE_SIZE, nullptr}},
                               nullptr);

        createPipelineLayout(m_descriptor_set_layout);

        auto [vertex_code, fragment_code] = common::shaders::read_spirv_files("draw_mesh");
        auto vertex_shader_module = common::shaders::create_shader_module(device, vertex_code);
        auto fragment_shader_module = common::shaders::create_shader_module(device, fragment_code);
        createPipeline(vertex_shader_module, fragment_shader_module);
    }

    ~DynamicDrawMeshPipeline() = default;

    [[nodiscard]] const vk::raii::Pipeline &handle() { return m_pipeline; }
    [[nodiscard]] const vk::raii::PipelineLayout &layout() { return m_pipeline_layout; }
    [[nodiscard]] const vk::raii::DescriptorSet &desc_set() { return m_descriptor_set; }

    DynamicDrawMeshPipeline(DynamicDrawMeshPipeline &&other) noexcept = default;

private:
    void createPipelineLayout(const vk::raii::DescriptorSetLayout &descriptorSetLayout) {
        vk::PipelineLayoutCreateInfo pipelineLayoutInfo({}, *descriptorSetLayout);
        m_pipeline_layout = vk::raii::PipelineLayout(m_device, pipelineLayoutInfo);
    }

    void createPipeline(vk::raii::ShaderModule &vertexShaderModule, vk::raii::ShaderModule &fragmentShaderModule) {
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

        vk::ColorComponentFlags colorComponentFlags(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                                    vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
        vk::PipelineColorBlendAttachmentState color_attachment_state(false,
                                                                     vk::BlendFactor::eZero,
                                                                     vk::BlendFactor::eZero,
                                                                     vk::BlendOp::eAdd,
                                                                     vk::BlendFactor::eZero,
                                                                     vk::BlendFactor::eZero,
                                                                     vk::BlendOp::eAdd,
                                                                     colorComponentFlags);

        const vk::PipelineColorBlendStateCreateInfo color_blend_state{{},
                                                                      false,
                                                                      vk::LogicOp::eNoOp,
                                                                      color_attachment_state,
                                                                      {{1.0f, 1.0f, 1.0f, 1.0f}}};

        vk::StencilOpState stencilOpState(vk::StencilOp::eKeep,
                                          vk::StencilOp::eKeep,
                                          vk::StencilOp::eKeep,
                                          vk::CompareOp::eAlways);
        vk::PipelineDepthStencilStateCreateInfo depth_stencil_state({},
                                                                    true,
                                                                    true,
                                                                    vk::CompareOp::eLessOrEqual,
                                                                    false,
                                                                    false,
                                                                    stencilOpState,
                                                                    stencilOpState);

        constexpr auto dynamic_state_enables = std::array{vk::DynamicState::eViewport, vk::DynamicState::eScissor};
        const auto dynamic_state_info = vk::PipelineDynamicStateCreateInfo{
                {},
                static_cast<std::uint32_t>(dynamic_state_enables.size()),
                dynamic_state_enables.data()};

        auto vertex_input_bindings = std::array{
                vk::VertexInputBindingDescription{0, sizeof(Vertex), vk::VertexInputRate::eVertex}};

        auto vertex_input_attributes = std::array{
                vk::VertexInputAttributeDescription{0, 0, vk::Format::eR32G32B32A32Sfloat, 0},
                vk::VertexInputAttributeDescription{1, 0, vk::Format::eR32G32B32A32Sfloat, 16}};

        auto vertexInputInfo = vk::PipelineVertexInputStateCreateInfo{
                {},
                static_cast<std::uint32_t>(vertex_input_bindings.size()),
                vertex_input_bindings.data(),
                static_cast<std::uint32_t>(vertex_input_attributes.size()),
                vertex_input_attributes.data()};

        const auto shader_stages = std::array{
                vk::PipelineShaderStageCreateInfo{{}, vk::ShaderStageFlagBits::eVertex, vertexShaderModule, "main"},
                vk::PipelineShaderStageCreateInfo{{},
                                                  vk::ShaderStageFlagBits::eFragment,
                                                  fragmentShaderModule,
                                                  "main"}};

        vk::PipelineMultisampleStateCreateInfo multisampling({}, vk::SampleCountFlagBits::e1);
        vk::PipelineViewportStateCreateInfo viewportState({}, 1, nullptr, 1, nullptr);

        vk::PipelineRenderingCreateInfoKHR renderingCreateInfoKHR({},
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
                                                                  {});

        vk::GraphicsPipelineCreateInfo pipelineInfo({},
                                                    shader_stages,
                                                    &vertexInputInfo,
                                                    &input_assembly_state,
                                                    nullptr,
                                                    &viewportState,
                                                    &rasterization_state,
                                                    &multisampling,
                                                    &depth_stencil_state,
                                                    &color_blend_state,
                                                    &dynamic_state_info,
                                                    *m_pipeline_layout);
        pipelineInfo.setPNext(&renderingCreateInfoKHR);

        m_pipeline = vk::raii::Pipeline(m_device, m_pipeline_cache, pipelineInfo);
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
