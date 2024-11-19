// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include <boost/describe/class.hpp>
#include <boost/json/fwd.hpp>
#include <glm/fwd.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/vec3.hpp>

namespace sm::arcane::scene {

enum class viewpoint_action_e { base, load, save };

struct viewpoint_s {
    glm::f64vec3 position = {};
    glm::f32quat orientation = {};

    BOOST_DESCRIBE_STRUCT(viewpoint_s, (), (position, orientation))
};

[[nodiscard]] bool already_exists_viewpoint() noexcept;

[[nodiscard]] viewpoint_s load_viewpoint_from_json();
void save_viewpoint_to_json(const viewpoint_s & /* viewpoint */);

} // namespace sm::arcane::scene