#include "game_object.hpp"

#include <utility>

#include <glm/geometric.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/trigonometric.hpp>

namespace sm::arcane::objects {

GameObject::GameObject(std::shared_ptr<primitive_graphics::Mesh> &&mesh)
    : m_mesh{std::move(mesh)},
      m_color{glm::f32vec3{1.0}} {}

void GameObject::set_position(const glm::f64vec3 &position) noexcept { m_transform.position = position; }

void GameObject::set_scale(const float scale) noexcept { m_transform.scale *= scale; }

void GameObject::set_scale(const glm::f32vec3 &scale) noexcept { m_transform.scale *= scale; }

void GameObject::set_orientation(const float degrees, const glm::f32vec3 &axis) noexcept {
    const auto new_rotation = glm::angleAxis(glm::radians(degrees), glm::normalize((axis)));

    if (m_transform.orientation == glm::f32quat{}) {
        m_transform.orientation = new_rotation;
    } else {
        m_transform.orientation = new_rotation * m_transform.orientation;
    }
}

} // namespace sm::arcane::objects