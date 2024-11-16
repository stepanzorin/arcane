// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include "cameras/camera.hpp"
#include "vulkan/device.hpp"
#include "window.hpp"

namespace sm::arcane::scene {

class Scene {
public:
    explicit Scene(const Window &window, const vulkan::Device &device, float swapchain_aspect_ratio);

    [[nodiscard]] cameras::Camera &camera();

    void update();

private:
    void update_camera_state();

    const Window &m_window;
    const vulkan::Device &m_device;

    cameras::Camera m_camera;
};

} // namespace sm::arcane::scene