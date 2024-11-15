// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

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


class Camera {
public:
    explicit Camera() = delete;
    explicit Camera(float swapchain_aspect_ratio);

    Camera(const Camera &) = delete;
    Camera &operator=(const Camera &) = delete;
    Camera(Camera &&) noexcept = delete;
    Camera &operator=(Camera &&) noexcept = delete;

    ~Camera() = default;

    [[nodiscard]] const camera_settings_s &settings() const { return m_settings; }
    [[nodiscard]] const camera_eye_s<double> &eye_d() const { return m_eye_d; }
    [[nodiscard]] const camera_matrices_s<double> &matrices() const { return m_matrices; }

    void set_position(const glm::f64vec3 &new_position);
    void set_orientation(float degrees, const glm::f32vec3 &axis) noexcept;
    void set_orientation(const glm::f64quat &new_orientation) noexcept;

    void move(movement_direction_e direction, float dt) noexcept;

    void loot_at(const glm::f64vec3 &target_position) noexcept;

    void update();

private:
    void update_eye_directions() noexcept;
    void update_projection_matrix() noexcept;
    void update_view_matrix() noexcept;

    float m_swapchain_aspect_ratio;

    camera_settings_s m_settings = {};
    camera_eye_s<double> m_eye_d = {};
    camera_matrices_s<double> m_matrices = {};
};

} // namespace sm::arcane::cameras