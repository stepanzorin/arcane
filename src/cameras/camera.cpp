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

void camera_s::update(const float swapchain_aspect_ratio) {
    auto &aspect_ratio = settings.aspect_ratio;
    auto &prev_aspect_ratio = settings.prev_aspect_ratio;

    prev_aspect_ratio = aspect_ratio;
    aspect_ratio = swapchain_aspect_ratio;

    if (aspect_ratio != prev_aspect_ratio) {
        eye_d.fov = compute_fov<double>(aspect_ratio);
        matrices = calculate_camera_matrices<double>(eye_d, aspect_ratio);
        update_eye_directions();
    }
}

void camera_s::update_eye_directions() noexcept {
    auto &[forward, up, right] = eye_d.transform.directions;
    const auto &view_matrix = matrices.view_matrix;

    forward = glm::normalize(glm::f32vec3{view_matrix[0][2], view_matrix[1][2], view_matrix[2][2]});
    right = glm::normalize(glm::f32vec3{view_matrix[0][0], view_matrix[1][0], view_matrix[2][0]});
    up = glm::cross(forward, right);
}

} // namespace sm::arcane::cameras