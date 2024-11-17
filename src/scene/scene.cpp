#include "scene.hpp"

#include <chrono>

#include "frame.hpp"

#include <spdlog/spdlog.h>

namespace sm::arcane::scene {

namespace {

[[nodiscard]] float calculate_dt(const frame_info_s frame_info) {
    return std::chrono::duration<float>{std::chrono::high_resolution_clock::now() - frame_info.started_time}.count();
}

} // namespace

Scene::Scene(Window &window, const vulkan::Device &device, const float swapchain_aspect_ratio)
    : m_window{window},
      m_device{device},
      m_camera{swapchain_aspect_ratio} {}

cameras::Camera &Scene::camera() { return m_camera; }

void Scene::update() {
    m_camera.update();
    update_camera_state();
}

void Scene::update_camera_state() {
    const auto dt = m_device.frame_dt();

    { // keyboard
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

        if (m_window.is_key_pressed(keyboard_key_e::q)) {
            m_camera.rotate(cameras::rotation_direction_e::left, dt);
        }

        if (m_window.is_key_pressed(keyboard_key_e::e)) {
            m_camera.rotate(cameras::rotation_direction_e::right, dt);
        }
    }

    { // mouse
        const auto &mouse = m_window.mouse();

        auto x_offset = static_cast<float>(mouse.dx) * dt;
        auto y_offset = static_cast<float>(mouse.dy) * dt;
        constexpr auto sensitivity = 80.0f;

        if (mouse.left_button_pressed && mouse.right_button_pressed) {
            x_offset *= sensitivity;
            m_camera.set_orientation(x_offset, glm::f32vec3{0.0f, 0.0f, 1.0f});
        } else if (mouse.left_button_pressed) {
            x_offset *= sensitivity;
            y_offset *= sensitivity;

            m_camera.set_orientation(x_offset, glm::f32vec3{0.0f, -1.0f, 0.0f});
            m_camera.set_orientation(y_offset, glm::f32vec3{-1.0f, 0.0f, 0.0f});
        }

        m_window.reset_mouse();
    }
}

} // namespace sm::arcane::scene