// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include <memory>
#include <optional>

#include <glm/fwd.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/vec3.hpp>

#include "cameras/transform.hpp"
#include "primitive_graphics/mesh.hpp"

namespace sm::arcane::objects {
class GameObject {
public:
    GameObject(const GameObject &) = delete;
    GameObject &operator=(const GameObject &) = delete;
    GameObject(GameObject &&) noexcept = default;
    GameObject &operator=(GameObject &&) noexcept = default;

    explicit GameObject(std::shared_ptr<primitive_graphics::Mesh> &&mesh);

    void set_position(const glm::f64vec3 &position) noexcept;
    void set_orientation(float degrees, const glm::f32vec3 &axis) noexcept;
    void set_scale(float scale) noexcept;
    void set_scale(const glm::f32vec3 &scale) noexcept;

    [[nodiscard]] cameras::transform_object_s transform() const noexcept;
    [[nodiscard]] glm::f32vec3 color() const noexcept;
    [[nodiscard]] std::shared_ptr<primitive_graphics::Mesh> mesh() const noexcept;

private:
    std::shared_ptr<primitive_graphics::Mesh> m_mesh;
    cameras::transform_object_s m_transform;

    std::optional<glm::f32vec3> m_color;
};

} // namespace sm::arcane::objects