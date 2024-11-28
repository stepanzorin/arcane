// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include <glm/fwd.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace sm::arcane::cameras {

enum class movement_direction_e { forward, backward, left, right, up, down };

enum class rotation_direction_e { forward, backward, left, right };


inline constexpr auto g_direction_forward = glm::f32vec3{0.0f, 0.0f, 1.0f};
inline constexpr auto g_direction_up = glm::f32vec3{0.0f, 1.0f, 0.0f};
inline constexpr auto g_direction_right = glm::f32vec3{1.0f, 0.0f, 0.0f};

inline constexpr auto g_default_position = glm::f64vec3{0.0, 0.0, 0.0};
inline constexpr auto g_default_orientation = glm::f32quat{0.0f, 0.0f, 0.0f, 0.0f};
inline constexpr auto g_default_scale = glm::f32vec3{1.0f, 1.0f, 1.0f};

struct pose_s {
    glm::f64vec3 position = g_default_position;
    glm::f32quat orientation = g_default_orientation;
};

struct transform_s : pose_s {
    struct directions_s {
        glm::f32vec3 forward = g_direction_forward;
        glm::f32vec3 up = g_direction_up;
        glm::f32vec3 right = g_direction_right;
    };

    directions_s directions;
};


struct transform_object_s : transform_s {
    glm::f32vec3 scale = g_default_scale;

    [[nodiscard]] glm::f64mat4 model_matrix() const noexcept;
    [[nodiscard]] glm::f32mat3 normal_matrix() const noexcept;
};

} // namespace sm::arcane::cameras