#include "scene.hpp"

#include "scene/viewpoint.hpp"

namespace sm::arcane::scene {

Scene::Scene(Window &window, const vulkan::Device &device, const std::unique_ptr<vulkan::Swapchain> &swapchain)
    : m_window{window},
      m_device{device},
      m_swapchain_uptr{swapchain},
      m_camera{swapchain->aspect_ratio()} {}

cameras::Camera &Scene::camera() { return m_camera; }

void Scene::update() { update_camera_state(); }

void Scene::update_camera_state() {
    m_camera.update(m_swapchain_uptr->aspect_ratio());

    const auto dt = m_device.frame_dt();

    { // mouse
        const auto &mouse = m_window.mouse();

        auto x_offset = static_cast<float>(mouse.dx) * dt;
        auto y_offset = static_cast<float>(mouse.dy) * dt;

        if (mouse.left_button_pressed && mouse.right_button_pressed) {
            x_offset *= mouse.config.sensitivity;
            m_camera.set_orientation(x_offset, glm::f32vec3{0.0f, 0.0f, 1.0f});
        } else if (mouse.left_button_pressed) {
            x_offset *= mouse.config.sensitivity;
            y_offset *= mouse.config.sensitivity;

            m_camera.set_orientation(x_offset, glm::f32vec3{0.0f, -1.0f, 0.0f});
            m_camera.set_orientation(y_offset, glm::f32vec3{-1.0f, 0.0f, 0.0f});
        }

        m_window.reset_mouse();
    }

    { // keyboard
        { // camera manipulation
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

        { // viewpoint manipulation
            if (m_window.is_key_pressed(keyboard_key_e::f9)) {
                m_camera.use_or_create_viewpoint(viewpoint_action_e::base);
            }

            if (m_window.is_key_pressed(keyboard_key_e::f10)) {
                m_camera.use_or_create_viewpoint(viewpoint_action_e::save);
            }

            if (m_window.is_key_pressed(keyboard_key_e::f11)) {
                m_camera.use_or_create_viewpoint(viewpoint_action_e::load);
            }
        }
    }
}

} // namespace sm::arcane::scene