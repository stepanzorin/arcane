#include "scene.hpp"

namespace sm::arcane::scene {

namespace {

[[nodiscard]] float calculate_dt(const render::frame_info_s frame_info) {
    return std::chrono::duration<float>{std::chrono::high_resolution_clock::now() - frame_info.started_time}.count();
}

} // namespace

Scene::Scene(const Window &window, const render::frame_info_s &frame_info, const float swapchain_aspect_ratio)
    : m_window{window},
      m_frame_info{frame_info},
      m_camera{swapchain_aspect_ratio} {}

cameras::Camera &Scene::camera() { return m_camera; }

void Scene::update() {
    m_camera.update();
    update_camera_state();
}

void Scene::update_camera_state() {
    const auto dt = calculate_dt(m_frame_info);

    if (m_window.is_key_pressed(keyboard_key_e::w)) {
        m_camera.move(cameras::movement_direction_e::forward, dt);
    }

    if (m_window.is_key_pressed(keyboard_key_e::s)) {
        m_camera.move(cameras::movement_direction_e::backward, dt);
    }

    if (m_window.is_key_pressed(keyboard_key_e::a)) {
        m_camera.move(cameras::movement_direction_e::left, dt);
    }

    if (m_window.is_key_pressed(keyboard_key_e::d)) {
        m_camera.move(cameras::movement_direction_e::right, dt);
    }

    if (m_window.is_key_pressed(keyboard_key_e::r)) {
        m_camera.move(cameras::movement_direction_e::up, dt);
    }

    if (m_window.is_key_pressed(keyboard_key_e::f)) {
        m_camera.move(cameras::movement_direction_e::down, dt);
    }
}

} // namespace sm::arcane::scene