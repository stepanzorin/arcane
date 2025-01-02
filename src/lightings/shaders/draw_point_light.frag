#version 450

layout(location = 0) in vec2 frag_offset;

layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 0) uniform global_ubo_s {
    mat4 projection;
    mat4 view;
    mat4 inverse_view;
    vec4 ambient_light_color;
    vec4 light_color;
    vec3 light_position;
}
global_ubo;

void main() {
    float dis = sqrt(dot(frag_offset, frag_offset));

    if (dis >= 1.0) {
        discard;
    }

    out_color = vec4(global_ubo.light_color.xyz, 1.0);
}