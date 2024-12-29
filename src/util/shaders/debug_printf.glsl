// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#ifndef SM_ARCANE_UTIL_SHADERS_DEBUG_PRINTF_GLSL
#define SM_ARCANE_UTIL_SHADERS_DEBUG_PRINTF_GLSL

// https://github.com/KhronosGroup/Vulkan-ValidationLayers/blob/main/docs/debug_printf.md

#extension GL_EXT_debug_printf : enable


void printf(const int value) { debugPrintfEXT("%i", value); }

void printf(const uint value) { debugPrintfEXT("%u", value); }

void printf(const float value) { debugPrintfEXT("%f", value); }


void printf(const vec2 vec) { debugPrintfEXT("vec2 = { %f, %f }", vec.x, vec.y); }

void printf(const vec3 vec) { debugPrintfEXT("vec3 = { %f, %f, %f }", vec.x, vec.y, vec.z); }

void printf(const vec4 vec) { debugPrintfEXT("vec4 = { %f, %f, %f, %f }", vec.x, vec.y, vec.z, vec.w); }

void printf(const ivec2 vec) { debugPrintfEXT("ivec2 = { %i, %i }", vec.x, vec.y); }

void printf(const ivec3 vec) { debugPrintfEXT("ivec3 = { %i, %i, %i }", vec.x, vec.y, vec.z); }

void printf(const ivec4 vec) { debugPrintfEXT("ivec4 = { %i, %i, %i, %i }", vec.x, vec.y, vec.z, vec.w); }

void printf(const uvec2 vec) { debugPrintfEXT("uvec2 = { %u, %u }", vec.x, vec.y); }

void printf(const uvec3 vec) { debugPrintfEXT("uvec3 = { %u, %u, %u }", vec.x, vec.y, vec.z); }

void printf(const uvec4 vec) { debugPrintfEXT("uvec4 = { %u, %u, %u, %u }", vec.x, vec.y, vec.z, vec.w); }


// clang-format off

void printf(const mat2 mat) {
    debugPrintfEXT("mat2 = {\n  { %f, %f }\n  { %f, %f }\n}",
                   mat[0][0], mat[0][1], mat[1][0], mat[1][1]);
}

void printf(const mat3 mat) {
    debugPrintfEXT("mat3 = {\n  { %f, %f, %f }\n  { %f, %f, %f }\n  { %f, %f, %f }\n}",
                   mat[0][0], mat[0][1], mat[0][2],
                   mat[1][0], mat[1][1], mat[1][2],
                   mat[2][0], mat[2][1], mat[2][2]);
}

void printf(const mat4 mat) {
    debugPrintfEXT("mat4 = {\n  { %f, %f, %f, %f }\n  { %f, %f, %f, %f }\n  { %f, %f, %f, %f }\n  { %f, %f, %f, %f }\n}",
                   mat[0][0], mat[0][1], mat[0][2], mat[0][3],
                   mat[1][0], mat[1][1], mat[1][2], mat[1][3],
                   mat[2][0], mat[2][1], mat[2][2], mat[2][3],
                   mat[3][0], mat[3][1], mat[3][2], mat[3][3]);
}

// clang-format on

#endif