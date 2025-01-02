// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2025 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include "lightings/shaders/draw_point_light_pipeline.hpp"
#include "primitive_graphics/mesh.hpp"
#include "render/common.hpp"
#include "vulkan/swapchain.hpp"

namespace sm::arcane::lightings {

class DrawPointLightSystem {
public:
    struct resources_s {
        shaders::DynamicDrawPointLightPipeline pipeline;

        [[nodiscard]] static resources_s create(const render::pass_context_s &ctx) {
            return {.pipeline = {ctx.device.device(),
                                 ctx.global.descriptor_set_layout,
                                 ctx.swapchain->color_format(),
                                 ctx.swapchain->depth_format()}};
        }
    };

    explicit DrawPointLightSystem(const render::pass_context_s &ctx) : m_resources{resources_s::create(ctx)} {}

    void render(const render::render_args_s &args) const {
        args.command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_resources.pipeline.handle());

        args.command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                               *m_resources.pipeline.layout(),
                                               0,
                                               {args.global.descriptor_set},
                                               nullptr);

        args.command_buffer.draw(6, 1, 0, 0);
    }

    resources_s m_resources;
};

} // namespace sm::arcane::lightings