#include "transform.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace sm::arcane::cameras {

namespace {

struct quaternion_transform_components_s {
    double xx, yy, zz;
    double xy, xz, yz;
    double wx, wy, wz;
};

[[nodiscard]] quaternion_transform_components_s calculate_quaternion_transform_components(
        const glm::f64quat &orientation) {
    return {.xx = orientation.x * orientation.x,
            .yy = orientation.y * orientation.y,
            .zz = orientation.z * orientation.z,
            .xy = orientation.x * orientation.y,
            .xz = orientation.x * orientation.z,
            .yz = orientation.y * orientation.z,
            .wx = orientation.w * orientation.x,
            .wy = orientation.w * orientation.y,
            .wz = orientation.w * orientation.z};
}

} // namespace

glm::f64mat4 transform_object_s::model_matrix() const noexcept {
    // clang-format off
    const auto [xx, yy, zz,
                xy, xz, yz,
                wx, wy, wz] = calculate_quaternion_transform_components(orientation);
    // clang-format on

    return {{scale.x * (1.0 - 2.0 * (yy + zz)), scale.x * (2.0 * (xy - wz)), scale.x * (2.0 * (xz + wy)), 0.0},
            {scale.y * (2.0 * (xy + wz)), scale.y * (1.0 - 2.0 * (xx + zz)), scale.y * (2.0 * (yz - wx)), 0.0},
            {scale.z * (2.0 * (xz - wy)), scale.z * (2.0 * (yz + wx)), scale.z * (1.0 - 2.0 * (xx + yy)), 0.0},
            {position.x, position.y, position.z, 1.0}};
}

glm::f32mat3 transform_object_s::normal_matrix() const noexcept {
    // clang-format off
    const auto [xx, yy, zz,
                xy, xz, yz,
                wx, wy, wz] = calculate_quaternion_transform_components(orientation);
    // clang-format on

    const auto inv_scale = glm::f32vec3{1.0f / scale};

    return {{inv_scale.x * (1.0f - 2.0f * (yy + zz)),
             inv_scale.x * (2.0f * (xy - wz)),
             inv_scale.x * (2.0f * (xz + wy))},
            {inv_scale.y * (2.0f * (xy + wz)),
             inv_scale.y * (1.0f - 2.0f * (xx + zz)),
             inv_scale.y * (2.0f * (yz - wx))},
            {inv_scale.z * (2.0f * (xz - wy)),
             inv_scale.z * (2.0f * (yz + wx)),
             inv_scale.z * (1.0f - 2.0f * (xx + yy))}};
}

} // namespace sm::arcane::cameras