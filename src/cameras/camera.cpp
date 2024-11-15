#include "camera.hpp"

#include <cmath>
#include <limits>
#include <numbers>

namespace sm::arcane::cameras {

namespace {

template<typename T>
[[nodiscard]] glm::tvec2<T, glm::qualifier::highp> compute_fov(const float aspect_ratio) noexcept {
    constexpr auto horizontal_fov = 0.25 * std::numbers::pi_v<double>;
    const auto vertical_fov = 2.0 * std::atan(std::tan(horizontal_fov / 2.0) * static_cast<double>(aspect_ratio));
    return {static_cast<T>(horizontal_fov), static_cast<T>(vertical_fov)};
}

template<typename T>
[[nodiscard]] camera_eye_s<T> create_camera_eye(camera_settings_s &settings) {
    auto transform = transform_s{};
    transform.position = {0.0f, 0.0f, 0.0f};
    transform.orientation = {0.0f, 0.0f, 0.0f, 0.0f};

    return {.transform = transform,
            .fov = compute_fov<double>(settings.aspect_ratio),
            .near_clip = 0.01,
            .far_clip = 1000.0};
}

template<typename T>
[[nodiscard]] glm::f64mat4 compute_perspective_projection_old(const float aspect,
                                                              const glm::tvec2<T, glm::qualifier::highp> &fov,
                                                              const T near,
                                                              const T far) noexcept {
    assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f && "Horizontal FOV cannot be close to zero");

    const auto tan_half_fov = std::tan(fov.x / 2.0);

    return {{1.0 / (aspect * tan_half_fov), 0.0, 0.0, 0.0},
            {0.0, 1.0 / tan_half_fov, 0.0, 0.0},
            {0.0, 0.0, far / (far - near), 1.0},
            {0.0, 0.0, -(far * near) / (far - near), 0.0}};
}

[[nodiscard]] glm::f64mat4 compute_view_matrix(const glm::f64vec3 &position,
                                               const glm::f64vec3 &forward,
                                               const glm::f64vec3 &up) noexcept {
    const auto w = glm::f64vec3{glm::normalize(forward)};
    const auto u = glm::f64vec3{glm::normalize(glm::cross(w, up))};
    const auto v = glm::f64vec3{glm::cross(w, u)};

    return {{u.x, v.x, w.x, 0.0},
            {u.y, v.y, w.y, 0.0},
            {u.z, v.z, w.z, 0.0},
            {-glm::dot(u, position), -glm::dot(v, position), -glm::dot(w, position), 1.0}};
}

template<typename T>
[[nodiscard]] glm::f64mat4 compute_view_matrix_YXZ(const glm::f64vec3 &position,
                                                   const glm::qua<T, glm::qualifier::highp> &rotation) noexcept {
    const auto xx = static_cast<double>(rotation.x * rotation.x);
    const auto yy = static_cast<double>(rotation.y * rotation.y);
    const auto zz = static_cast<double>(rotation.z * rotation.z);
    const auto xy = static_cast<double>(rotation.x * rotation.y);
    const auto xz = static_cast<double>(rotation.x * rotation.z);
    const auto yz = static_cast<double>(rotation.y * rotation.z);
    const auto wx = static_cast<double>(rotation.w * rotation.x);
    const auto wy = static_cast<double>(rotation.w * rotation.y);
    const auto wz = static_cast<double>(rotation.w * rotation.z);

    const auto u = glm::f64vec3{(1.0 - 2.0 * (yy + zz)), (2.0 * (xy - wz)), (2.0 * (xz + wy))};
    const auto v = glm::f64vec3{(2.0 * (xy + wz)), (1.0 - 2.0 * (xx + zz)), (2.0 * (yz - wx))};
    const auto w = glm::f64vec3{(2.0 * (xz - wy)), (2.0 * (yz + wx)), (1.0 - 2.0 * (xx + yy))};

    return {{u.x, v.x, w.x, 0.0},
            {u.y, v.y, w.y, 0.0},
            {u.z, v.z, w.z, 0.0},
            {-glm::dot(u, position), -glm::dot(v, position), -glm::dot(w, position), 1.0}};
}

template<typename T>
[[nodiscard]] glm::tmat4x4<T, glm::qualifier::highp> flip_projection_matrix(
        const glm::tmat4x4<T, glm::qualifier::highp> &updated_projection_matrix) noexcept {
    auto flipped = updated_projection_matrix;
    flipped[1][1] *= -1.0f;
    return flipped;
}

template<typename T>
[[nodiscard]] camera_matrices_s<T> calculate_camera_projection_matrix(const camera_eye_s<T> &eye,
                                                                      const float aspect_ratio) noexcept {
    auto matrices = camera_matrices_s<T>{};
    matrices.projection_matrix = compute_perspective_projection_old(aspect_ratio, eye.fov, eye.near_clip, eye.far_clip);
    matrices.projection_matrix_flipped = flip_projection_matrix(matrices.projection_matrix);
    matrices.projection_matrix_inverted = glm::inverse(matrices.projection_matrix);
    return matrices;
}

template<typename T>
[[nodiscard]] camera_matrices_s<T> calculate_camera_view_matrix(const camera_eye_s<T> &eye) noexcept {
    auto matrices = camera_matrices_s<T>{};
    matrices.view_matrix = compute_view_matrix_YXZ(eye.transform.position, eye.transform.orientation);
    matrices.view_matrix_inverted = glm::inverse(matrices.view_matrix);
    return matrices;
}

template<typename T>
[[nodiscard]] camera_matrices_s<T> calculate_camera_matrices(const camera_eye_s<T> &eye,
                                                             const float aspect_ratio) noexcept {
    // only   1/5, 2/5, 3/5   are filled
    auto projection_matrices = calculate_camera_projection_matrix(eye, aspect_ratio);

    // only   4/5, 5/5    are filled
    auto view_matrices = calculate_camera_view_matrix(eye);

    return {.projection_matrix = projection_matrices.projection_matrix,
            .projection_matrix_flipped = projection_matrices.projection_matrix_flipped,
            .projection_matrix_inverted = projection_matrices.projection_matrix_inverted,
            .view_matrix = view_matrices.view_matrix,
            .view_matrix_inverted = view_matrices.view_matrix_inverted};
}

} // namespace

Camera::Camera(const float swapchain_aspect_ratio)
    : m_swapchain_aspect_ratio{swapchain_aspect_ratio},
      m_settings{camera_settings_s{m_swapchain_aspect_ratio}},
      m_eye_d{create_camera_eye<double>(m_settings)},
      m_matrices{calculate_camera_matrices<double>(m_eye_d, m_settings.aspect_ratio)} {}


void Camera::loot_at(const glm::f64vec3 &target_position) noexcept {
    m_matrices.view_matrix = compute_view_matrix(m_eye_d.transform.position,
                                                 target_position - m_eye_d.transform.position,
                                                 m_eye_d.transform.directions.up);
    update_view_matrix();
}

void Camera::update() {
    auto &aspect_ratio = m_settings.aspect_ratio;
    auto &prev_aspect_ratio = m_settings.prev_aspect_ratio;

    prev_aspect_ratio = aspect_ratio;
    aspect_ratio = m_swapchain_aspect_ratio;

    if (aspect_ratio != prev_aspect_ratio) {
        m_eye_d.fov = compute_fov<double>(aspect_ratio);
        m_matrices = calculate_camera_matrices<double>(m_eye_d, aspect_ratio);
        update_eye_directions();
    }
}

void Camera::update_projection_matrix() noexcept {
    m_matrices.projection_matrix = compute_perspective_projection_old(m_settings.aspect_ratio,
                                                                      m_eye_d.fov,
                                                                      m_eye_d.near_clip,
                                                                      m_eye_d.far_clip);
    m_matrices.projection_matrix_flipped = flip_projection_matrix(m_matrices.projection_matrix);
    m_matrices.projection_matrix_inverted = glm::inverse(m_matrices.projection_matrix);
}

void Camera::update_view_matrix() noexcept {
    m_matrices.view_matrix = compute_view_matrix_YXZ(m_eye_d.transform.position, m_eye_d.transform.orientation);
    m_matrices.view_matrix_inverted = glm::inverse(m_matrices.view_matrix);
}

void Camera::update_eye_directions() noexcept {
    auto &[forward, up, right] = m_eye_d.transform.directions;
    const auto &view_matrix = m_matrices.view_matrix;

    forward = glm::normalize(glm::f32vec3(view_matrix[0][2], view_matrix[1][2], view_matrix[2][2]));
    right = glm::normalize(glm::f32vec3(view_matrix[0][0], view_matrix[1][0], view_matrix[2][0]));
    up = glm::cross(forward, right);
}

void Camera::set_position(const glm::f64vec3 &new_position) {
    m_eye_d.transform.position = new_position;
    update_view_matrix();
}

void Camera::set_orientation(const float degrees, const glm::f32vec3 &axis) noexcept {
    const auto new_orientation = glm::angleAxis(glm::radians(degrees), axis);
    set_orientation(glm::normalize(new_orientation * m_eye_d.transform.orientation));
    update_view_matrix();
    update_eye_directions();
}

void Camera::set_orientation(const glm::f64quat &new_orientation) noexcept {
    m_eye_d.transform.orientation = new_orientation;
}

void Camera::move(const movement_direction_e direction, const float dt) noexcept {
    auto move_direction = glm::f32vec3{};

    const auto &[forward, up, right] = m_eye_d.transform.directions;

    if (direction == movement_direction_e::forward) {
        move_direction += forward;
    }

    if (direction == movement_direction_e::backward) {
        move_direction -= forward;
    }

    if (direction == movement_direction_e::up) {
        move_direction -= up;
    }

    if (direction == movement_direction_e::down) {
        move_direction += up;
    }

    if (direction == movement_direction_e::left) {
        move_direction -= right;
    }

    if (direction == movement_direction_e::right) {
        move_direction += right;
    }

    move_direction *= m_settings.movement_speed;
    const auto new_position = m_eye_d.transform.position +
                              static_cast<glm::f64vec3>(move_direction) * static_cast<double>(dt);

    set_position(new_position);
}

} // namespace sm::arcane::cameras