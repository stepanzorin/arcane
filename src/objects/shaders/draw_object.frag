#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec3 fragment_position_world;
layout(location = 1) in vec4 fragment_color;
layout(location = 2) in vec3 fragment_normal_color;

layout(location = 0) out vec4 albedo;

layout(set = 0, binding = 0) uniform global_ubo_s {
    mat4 projection;
    mat4 view;
}
global_ubo;

layout(push_constant) uniform push_s {
    mat4 model_matrix;
    mat4 normal_matrix;
}
push;

void main() {
    vec4 global_ubo_ambient_light_color = vec4(1.0);
    vec3 global_ubo_light_position = vec3(1.0);
    vec4 global_ubo_light_color = vec4(1.0);

    vec3 direction_to_light = global_ubo_light_position - fragment_position_world;
    float attenuation = 1.0 / dot(direction_to_light, direction_to_light);

    vec3 light_color = global_ubo_light_color.xyz * global_ubo_light_color.w * attenuation;
    vec3 ambient_light = global_ubo_ambient_light_color.xyz * global_ubo_ambient_light_color.w;
    vec3 diffuse_light = light_color * max(dot(normalize(fragment_normal_color), normalize(direction_to_light)), 0);

    albedo = vec4(((diffuse_light + ambient_light) * fragment_color.xyz) * vec3(1.0), 1.0);
}