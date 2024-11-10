#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec4 vertex_color;

layout(location = 0) out vec4 fragment_color;

layout(set = 0, binding = 0) uniform global_ubo_s {
    mat4 projection;
    mat4 view;
}
global_ubo;

void main() {
    // vec4 position_world = push.model_matrix * vec4(vertex_position, 1.0); TODO: need to support `push constants`

    mat4 model_matrix = mat4(1.0);
    model_matrix[3][2] = 5.0;

    vec4 position_world = model_matrix * vec4(vertex_position, 1.0);

    gl_Position = global_ubo.projection * global_ubo.view * position_world;

    fragment_color = vertex_color;
}