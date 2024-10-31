// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include <array>

#include <glm/fwd.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace sm::arcane::primitive_graphics {

struct box_s {
    glm::f64vec3 center_position;
    glm::f32vec3 ax;
    glm::f32vec3 ay;
    glm::f32vec3 az;
    glm::f32vec3 extent;
    glm::f32vec4 color;
};

struct cone_s {
    glm::f64vec3 center_position;
    glm::f32vec3 direction;
    float length;
    float radius_base;
    glm::f32vec4 color;
};

struct cylinder_s {
    glm::f64vec3 center_position;
    glm::f32vec3 direction;
    float length;
    float radius;
    glm::f32vec4 color;
};
using capsule_s = cylinder_s;

struct ellipsoid_s {
    glm::f64vec3 center_position;
    glm::f32vec3 radius;
    glm::f32vec4 color;
};

struct frustum_cone_s {
    glm::f64vec3 center_position;
    glm::f32vec3 direction;
    float length;
    float radius_top;
    float radius_base;
    glm::f32vec4 color;
};

struct line_s {
    std::array<glm::f64vec3, 2> position;
    float width;
    glm::f32vec4 color;
};

struct pyramid_s {
    glm::f64vec3 base_center_position;
    std::array<glm::f32vec3, 4> base_vertices;
    glm::f32vec3 direction;
    glm::f32vec3 apex_position;
    glm::f32vec4 color;
};

struct sphere_s {
    glm::f64vec3 center_position;
    float radius;
    glm::f32vec4 color;
};

struct torus_s {
    glm::f64vec3 center_position;
    glm::f32vec3 direction;
    float major_radius;
    float minor_radius;
    glm::f32vec4 color;
};

struct quad_s {
    std::array<glm::f64vec3, 4> position;
    glm::f32vec4 color;
};

} // namespace sm::arcane::primitive_graphics