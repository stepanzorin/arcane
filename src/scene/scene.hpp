// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include <memory>

#include "cameras/camera.hpp"
#include "vulkan/device.hpp"
#include "vulkan/swapchain.hpp"
#include "window.hpp"

namespace sm::arcane::scene {

class Scene {
public:
    explicit Scene(Window &window, const vulkan::Device &device, const std::unique_ptr<vulkan::Swapchain> &swapchain);

    [[nodiscard]] cameras::Camera &camera();

    void update();

private:
    void update_camera_state();

    Window &m_window;
    const vulkan::Device &m_device;
    const std::unique_ptr<vulkan::Swapchain> &m_swapchain_uptr;

    cameras::Camera m_camera;
};

} // namespace sm::arcane::scene