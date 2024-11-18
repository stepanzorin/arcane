// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include <cstdint>
#include <memory>
#include <string_view>

#include <vulkan/vulkan.h>

#include "app_config.hpp"
#include "peripherals.hpp"

namespace sm::arcane {

// TODO: will define `#if ...` relative OS: Windows - WinAPI; Other - GLFW

// GLFW Macro Definition
// https://www.glfw.org/docs/3.4/group__input.html#gada11d965c4da13090ad336e030e4d11f

enum class event_type_e : std::int8_t { release = 0, press = 1, repeat = 2 };

// GLFW Mouse buttons
// https://www.glfw.org/docs/3.4/group__buttons.html

enum class mouse_button_e {
    left = 0,
    right = 1,
    middle = 2,
    xbutton4 = 3,
    xbutton5 = 4,
    xbutton6 = 5,
    xbutton7 = 6,
    xbutton8 = 7
};

// GLFW Keyboard keys
// https://www.glfw.org/docs/3.4/group__keys.html

enum class keyboard_key_e : std::int16_t {
    unknown = -1,

    space = 32,

    _0 = 48,
    _1 = 49,
    _2 = 50,
    _3 = 51,
    _4 = 52,
    _5 = 53,
    _6 = 54,
    _7 = 55,
    _8 = 56,
    _9 = 57,

    a = 65,
    b = 66,
    c = 67,
    d = 68,
    e = 69,
    f = 70,
    g = 71,
    h = 72,
    i = 73,
    j = 74,
    k = 75,
    l = 76,
    m = 77,
    n = 78,
    o = 79,
    p = 80,
    q = 81,
    r = 82,
    s = 83,
    t = 84,
    u = 85,
    v = 86,
    w = 87,
    x = 88,
    y = 89,
    z = 90,

    lbracket = 91,
    rbracket = 93,
    oem_grave = 96,
    oem_102 = 162, // Specific key in non-US layouts

    escape = 256,
    enter = 257,
    tab = 258,
    backspace = 259,

    insert = 260,
    del = 261,
    right = 262,
    left = 263,
    down = 264,
    up = 265,
    pageup = 266,
    pagedown = 267,
    home = 268,
    end = 269,

    f1 = 290,
    f2 = 291,
    f3 = 292,
    f4 = 293,
    f5 = 294,
    f6 = 295,
    f7 = 296,
    f8 = 297,
    f9 = 298,
    f10 = 299,
    f11 = 300,
    f12 = 301,
    f13 = 302,
    f14 = 303,
    f15 = 304,
    f16 = 305,
    f17 = 306,
    f18 = 307,
    f19 = 308,
    f20 = 309,
    f21 = 310,
    f22 = 311,
    f23 = 312,
    f24 = 313,

    kp_0 = 320,
    kp_1 = 321,
    kp_2 = 322,
    kp_3 = 323,
    kp_4 = 324,
    kp_5 = 325,
    kp_6 = 326,
    kp_7 = 327,
    kp_8 = 328,
    kp_9 = 329,
    decimal = 330,
    divide = 331,
    multiply = 332,
    subtract = 333,
    add = 334,

    shift = 340,
    ctrl = 341,
    alt = 342,
    lsuper = 343,
    rsuper = 347,

    max_key = 348 // The last recognized key
};

struct required_instance_extensions_s {
    std::uint32_t count;
    const char **extensions;
};

class Window {
public:
    Window() = delete;
    explicit Window(const app_config_s &config);

    Window(const Window &) = delete;
    Window &operator=(const Window &) = delete;
    Window(Window &&) noexcept = delete;
    Window &operator=(Window &&) noexcept = delete;

    [[nodiscard]] required_instance_extensions_s required_instance_extensions() const noexcept;
    [[nodiscard]] std::string_view title() const noexcept;
    [[nodiscard]] window_extent_s extent() const noexcept;
    [[nodiscard]] bool should_close() const noexcept;
    void pool_events() const noexcept;

    [[nodiscard]] VkSurfaceKHR create_surface(const VkInstance &) const;

    [[nodiscard]] bool is_resized() const noexcept;
    void reset_resize_state() noexcept;

    [[nodiscard]] bool is_key_pressed(keyboard_key_e key) const noexcept;
    [[nodiscard]] bool is_key_released(keyboard_key_e key) const noexcept;

    [[nodiscard]] const mouse_s &mouse() const noexcept;
    void reset_mouse() const noexcept;

    ~Window();

private:
    class Impl;
    std::unique_ptr<Impl> m_pimpl;
};

} // namespace sm::arcane