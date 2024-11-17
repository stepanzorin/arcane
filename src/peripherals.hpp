// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace sm::arcane {

struct keyboard_s {
    static constexpr auto key_count = static_cast<std::size_t>(348); // TODO: will define `#if ...` relative OS
    std::array<bool, key_count> keys = {};
    std::int32_t last_pressed_key_code = -1; // TODO: will define `#if ...` relative OS
    bool shift_pressed = false;
    bool ctrl_pressed = false;
    bool alt_pressed = false;
};

struct mouse_s {
    double last_x_position = 0.0;
    double last_y_position = 0.0;
    double previous_x_position = 0.0;
    double previous_y_position = 0.0;
    double dx = 0.0;
    double dy = 0.0;

    bool left_button_pressed = false;
    bool middle_button_pressed = false;
    bool right_button_pressed = false;

    std::int32_t wheel_delta = 0;

    void reset() noexcept {
        dx = 0;
        dy = 0;
        wheel_delta = 0;
    }
};

} // namespace sm::arcane