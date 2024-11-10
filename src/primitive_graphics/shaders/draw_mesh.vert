#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec3 vert_pos;
layout(location = 1) in vec4 vert_color;

layout(location = 0) out vec4 frag_color;

layout(std140, binding = 0) uniform ubo_s { mat4 mvp; }
ubo;

void main() {
    gl_Position = ubo.mvp * vec4(vert_pos, 1.0);
    frag_color = vert_color;
}