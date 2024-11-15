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

struct pose_s {
    glm::f64vec3 position = {0.0, 0.0, 0.0};
    glm::f32quat orientation = {0.0f, 0.0f, 0.0f, 0.0f};
};

struct transform_s : pose_s {
    struct directions_s {
        glm::f32vec3 forward = {0.0f, 0.0f, 1.0f};
        glm::f32vec3 up = {0.0f, 1.0f, 0.0f};
        glm::f32vec3 right = {1.0f, 0.0f, 0.0f};
    };

    directions_s directions;
};

struct transform_object_s : transform_s {
    glm::f32vec3 scale = {1.0f, 1.0f, 1.0f};

    [[nodiscard]] glm::f64mat4 model_matrix() const noexcept;
    [[nodiscard]] glm::f32mat3 normal_matrix() const noexcept;
};

} // namespace sm::arcane::cameras