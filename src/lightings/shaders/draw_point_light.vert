#version 450

const vec2 offsets[6] = vec2[](vec2(-1.0, -1.0),
                               vec2(-1.0, 1.0),
                               vec2(1.0, -1.0),
                               vec2(1.0, -1.0),
                               vec2(-1.0, 1.0),
                               vec2(1.0, 1.0));

layout(location = 0) out vec2 frag_offset;

layout(set = 0, binding = 0) uniform global_ubo_s {
    mat4 projection;
    mat4 view;
    mat4 inverse_view;
    vec4 ambient_light_color;
    vec4 light_color;
    vec3 light_position;
}
global_ubo;

const float light_raduis = 1.00;

void main() {
    frag_offset = offsets[gl_VertexIndex];

    vec3 camera_right_wolrd = {global_ubo.view[0][0], global_ubo.view[1][0], global_ubo.view[2][0]};
    vec3 camera_up_wolrd = {global_ubo.view[0][1], global_ubo.view[1][1], global_ubo.view[2][1]};

    vec3 position_world = global_ubo.light_position.xyz + light_raduis * frag_offset.x * camera_right_wolrd +
                          light_raduis * frag_offset.y * camera_up_wolrd;

    gl_Position = global_ubo.projection * global_ubo.view * vec4(position_world, 1.0);
}