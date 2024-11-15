// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include "cameras/camera.hpp"
#include "render/common.hpp"
#include "window.hpp"

namespace sm::arcane::scene {

class Scene {
public:
    explicit Scene(const Window &window, const render::frame_info_s &frame_info, const float swapchain_aspect_ratio);


    [[nodiscard]] cameras::Camera &camera();

    void update();

    void update_camera_state();

private:
    const Window &m_window;
    const render::frame_info_s &m_frame_info;
    cameras::Camera m_camera;
};

} // namespace sm::arcane::scene