#pragma once

#include <cstdint>

#include <vulkan/vulkan_raii.hpp>

#include "common/shaders/pipeline_functions.hpp"
#include "primitive_graphics/mesh.hpp"
#include "vulkan/image_barriers.hpp"

namespace sm::arcane::primitive_graphics::shaders {

class DynamicDrawMeshPipeline {
public:
    DynamicDrawMeshPipeline(const vk::raii::Device &device,
                            const vk::DescriptorSetLayout descriptor_set_layout,
                            const vk::Format color_format,
                            const vk::Format depth_format)
        : m_device(device),
          m_pipeline_layout(vk::raii::PipelineLayout(m_device, {{}, descriptor_set_layout})),
          m_pipeline_cache{m_device, vk::PipelineCacheCreateInfo{}},
          m_pipeline{nullptr},
          m_color_format{color_format},
          m_depth_format{depth_format} {
        createPipeline(m_device);
    }

    ~DynamicDrawMeshPipeline() = default;

    [[nodiscard]] const vk::raii::Pipeline &handle() const noexcept { return m_pipeline; }
    [[nodiscard]] const vk::raii::PipelineLayout &layout() const noexcept { return m_pipeline_layout; }

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
                                                                               vk::CullModeFlagBits::eFront,
                                                                               vk::FrontFace::eCounterClockwise);

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
                vk::VertexInputBindingDescription{0, sizeof(Mesh::vertex_s), vk::VertexInputRate::eVertex}};

        constexpr auto vertex_input_attributes = std::array{
                vk::VertexInputAttributeDescription{0, 0, vk::Format::eR32G32B32Sfloat, 0},
                vk::VertexInputAttributeDescription{1, 0, vk::Format::eR32G32B32A32Sfloat, 12}};

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

    vk::raii::PipelineLayout m_pipeline_layout;
    vk::raii::PipelineCache m_pipeline_cache;
    vk::raii::Pipeline m_pipeline;

    vk::Format m_color_format;
    vk::Format m_depth_format;
};

} // namespace sm::arcane::primitive_graphics::shaders
