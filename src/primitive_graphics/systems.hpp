// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include <utility>

#include "primitive_graphics/mesh.hpp"
#include "primitive_graphics/shaders/draw_mesh_pipeline.hpp"
#include "render/common.hpp"
#include "vulkan/swapchain.hpp"

namespace sm::arcane::primitive_graphics {

class DrawMeshSystem {
public:
    struct resources_s {
        shaders::DynamicDrawMeshPipeline draw_mesh_pipeline;

        Mesh cube_mesh;

        [[nodiscard]] static resources_s create(const render::pass_context_s &ctx) {
            auto vertices = blanks::cube_vertices;
            auto indices = blanks::cube_indices;

            return {.draw_mesh_pipeline = {ctx.device.device(),
                                           ctx.global.descriptor_set_layout,
                                           ctx.swapchain->color_format(),
                                           ctx.swapchain->depth_format()},
                    .cube_mesh = Mesh{ctx.device,
                                      *ctx.swapchain->command_pool(),
                                      std::move(vertices),
                                      std::move(indices)}};
        }
    };

    explicit DrawMeshSystem(const render::pass_context_s &ctx) : m_resources{resources_s::create(ctx)} {}

    void render(const render::render_args_s &args) const {
        args.command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_resources.draw_mesh_pipeline.handle());

        args.command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                               *m_resources.draw_mesh_pipeline.layout(),
                                               0,
                                               {args.global.descriptor_set},
                                               nullptr);

        m_resources.cube_mesh.bind(args.command_buffer);
        m_resources.cube_mesh.draw(args.command_buffer);
    }

    resources_s m_resources;
};

} // namespace sm::arcane::primitive_graphics