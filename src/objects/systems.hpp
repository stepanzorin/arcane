// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include <memory>
#include <utility>

#include "objects/game_object.hpp"
#include "objects/shaders/draw_object_pipeline.hpp"
#include "primitive_graphics/mesh.hpp"
#include "render/common.hpp"

namespace sm::arcane::objects {

class DrawGameObjectSystem {
public:
    struct resources_s {
        shaders::DynamicDrawObjectPipeline draw_object_pipeline;

        GameObject game_object;

        [[nodiscard]] static resources_s create(const render::pass_context_s &ctx) {
            auto vertices = primitive_graphics::blanks::cube_normal_vertices;
            auto indices = primitive_graphics::blanks::cube_indices;

            auto mesh = std::make_shared<primitive_graphics::Mesh>(ctx.device,
                                                                   *ctx.swapchain->command_pool(),
                                                                   std::move(vertices),
                                                                   std::move(indices));

            return {.draw_object_pipeline = {ctx.device.device(),
                                             ctx.global.descriptor_set_layout,
                                             ctx.swapchain->color_format(),
                                             ctx.swapchain->depth_format()},
                    .game_object = GameObject{std::move(mesh)}};
        }
    };

    explicit DrawGameObjectSystem(const render::pass_context_s &ctx) : m_resources{resources_s::create(ctx)} {}

    void render(const render::render_args_s &args) const {
        args.command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_resources.draw_object_pipeline.handle());

        args.command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                               *m_resources.draw_object_pipeline.layout(),
                                               0,
                                               {args.global.descriptor_set},
                                               nullptr);

        m_resources.game_object.mesh()->bind(args.command_buffer);
        m_resources.game_object.mesh()->draw(args.command_buffer);
    }

    resources_s m_resources;
};

} // namespace sm::arcane::objects