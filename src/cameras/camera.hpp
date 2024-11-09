// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2023-2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include <glm/fwd.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "cameras/transform.hpp"

namespace sm::arcane::cameras {

template<typename T>
struct camera_pose_s : pose_s {
    glm::tvec3<T, glm::qualifier::highp> pivot_distance = {0.0, 0.0, 0.0};
};

template<typename T>
struct camera_matrices_s {
    glm::tmat4x4<T, glm::qualifier::highp> projection_matrix;
    glm::tmat4x4<T, glm::qualifier::highp> projection_matrix_flipped;
    glm::tmat4x4<T, glm::qualifier::highp> projection_matrix_inverted;
    glm::tmat4x4<T, glm::qualifier::highp> view_matrix;
    glm::tmat4x4<T, glm::qualifier::highp> view_matrix_inverted;
};

struct camera_settings_s {
    float aspect_ratio;
    float prev_aspect_ratio;
    float movement_speed = 1.0f;
    float rotation_speed = 50.0f;
};

template<typename T>
struct camera_eye_s {
    transform_s transform;
    glm::tvec2<T, glm::qualifier::highp> fov;

    T near_clip;
    T far_clip;
};

struct camera_s {
    camera_settings_s settings = {};
    camera_eye_s<double> eye_d = {};
    camera_matrices_s<double> matrices = {};

    void update(float swapchain_aspect_ratio);

private:
    void update_eye_directions() noexcept;
};

} // namespace sm::arcane::cameras