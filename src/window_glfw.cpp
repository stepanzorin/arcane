#include "window.hpp"

#include <cstdint>
#include <stdexcept>
#include <string_view>
#include <utility>

#include <fmt/format.h>
#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

#include "peripherals.hpp"
#include "window.hpp"

namespace sm::arcane {

namespace {

#if SM_ARCANE_OPERATING_SYSTEM_WINDOWS
inline constexpr auto g_window_title_bar_y_offset = 31;
#endif

template<typename GLFWFn, typename... FnArgs>
[[nodiscard]] decltype(auto) throw_if_glfw_failed(const std::string_view exception_message,
                                                  GLFWFn &&glfw_fn,
                                                  FnArgs &&...fn_args) {
    auto *ptr = glfw_fn(std::forward_like<FnArgs>(fn_args)...);

    if (!ptr) {
        throw std::runtime_error{exception_message.data()};
    }

    return ptr;
}

[[nodiscard]] GLFWmonitor *detect_monitor() {
    return throw_if_glfw_failed("Failed to detect a monitor", glfwGetPrimaryMonitor);
}

[[nodiscard]] GLFWwindow *create_windowed_window(const std::string_view title, const window_config_s &config) {
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_FALSE);
    auto *window_ptr = throw_if_glfw_failed("Failed to create window window",
                                            glfwCreateWindow,
                                            config.extent.width,
                                            config.extent.height,
                                            title.data(),
                                            nullptr,
                                            nullptr);

#if SM_ARCANE_OPERATING_SYSTEM_WINDOWS
    glfwSetWindowPos(window_ptr, config.position.x, config.position.y + g_window_title_bar_y_offset);
#else
    glfwSetWindowPos(window_ptr, config.position.x, config.position.y);
#endif

    return window_ptr;
}

[[nodiscard]] GLFWwindow *create_maximized_window(const std::string_view title, const window_config_s &config) {
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    return throw_if_glfw_failed("Failed to create maximized window",
                                glfwCreateWindow,
                                config.extent.width,
                                config.extent.height,
                                title.data(),
                                nullptr,
                                nullptr);
}

[[nodiscard]] GLFWwindow *create_fullscreen_window(GLFWmonitor *monitor, const std::string_view title) {
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_FALSE);
    const auto *mode_ptr = glfwGetVideoMode(monitor);
    return throw_if_glfw_failed("Failed to create fullscreen window",
                                glfwCreateWindow,
                                mode_ptr->width,
                                mode_ptr->height,
                                title.data(),
                                monitor,
                                nullptr);
}

[[nodiscard]] GLFWwindow *create_window(GLFWmonitor *monitor,
                                        const std::string_view title,
                                        const window_config_s &config) {
    switch (config.mode) {
        case window_mode_e::windowed: return create_windowed_window(title, config);
        case window_mode_e::maximized: return create_maximized_window(title, config);
        case window_mode_e::fullscreen: return create_fullscreen_window(monitor, title);
    }
    std::unreachable();
}

class GLFWInitializer {
public:
    GLFWInitializer() {
        if (!glfwInit()) {
            throw std::runtime_error{"Failed to initialize GLFW"};
        }
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        glfwSetErrorCallback([](const std::int32_t error, const char *description) {
            throw std::runtime_error{fmt::format("GLFWInitializer: Error #{}: {}", error, description)};
        });

        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    }

    virtual ~GLFWInitializer() { glfwTerminate(); }
};

} // namespace

class Window::Impl : public GLFWInitializer {
public:
    explicit Impl(const app_config_s &config)
        : m_title{config.title},
          m_config{config.window_config},
          m_monitor_ptr{detect_monitor()},
          m_window_ptr{create_window(m_monitor_ptr, m_title, m_config)} {
        glfwSetWindowUserPointer(m_window_ptr, this);
        glfwSetFramebufferSizeCallback(m_window_ptr, resize_callback);
        glfwSetKeyCallback(m_window_ptr, keyboard_callback);
        glfwSetMouseButtonCallback(m_window_ptr, mouse_button_callback);
        glfwSetCursorPosCallback(m_window_ptr, mouse_movement_callback);
    }

    [[nodiscard]] required_instance_extensions_s required_instance_extensions() const noexcept {
        auto glfw_extensions_count = 0u;
        auto **glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extensions_count);
        return {.count = glfw_extensions_count, .extensions = glfw_extensions};
    }

    [[nodiscard]] bool should_close() const noexcept { return glfwWindowShouldClose(m_window_ptr); }

    [[nodiscard]] std::string_view title() const noexcept { return m_title; }

    [[nodiscard]] window_extent_s extent() const noexcept { return m_config.extent; }

    void pool_events() noexcept { return glfwPollEvents(); }

    [[nodiscard]] VkSurfaceKHR create_surface(const VkInstance &instance) const {
        auto surface = VkSurfaceKHR{};
        if (glfwCreateWindowSurface(instance, m_window_ptr, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create window surface");
        }
        return surface;
    }

    [[nodiscard]] bool is_key_pressed(const keyboard_key_e key) const noexcept {
        return m_keyboard.keys[std::to_underlying(key)];
    }

    [[nodiscard]] bool is_resized() const noexcept { return m_is_resized; }

    void reset_resize_state() noexcept { m_is_resized = false; }

    [[nodiscard]] bool is_key_released(const keyboard_key_e key) const noexcept {
        return !m_keyboard.keys[std::to_underlying(key)];
    }

    [[nodiscard]] const mouse_s &mouse() const noexcept { return m_mouse; }

    void reset_mouse() const noexcept { m_mouse.reset(); }

    ~Impl() override { glfwDestroyWindow(m_window_ptr); }

private:
    static void resize_callback(GLFWwindow *window, std::int32_t width, std::int32_t height);
    static void keyboard_callback(GLFWwindow *window,
                                  std::int32_t key,
                                  std::int32_t,
                                  std::int32_t action,
                                  std::int32_t mods);
    static void mouse_movement_callback(GLFWwindow *window, double xpos, double ypos);
    static void mouse_button_callback(GLFWwindow *window, std::int32_t button, std::int32_t action, std::int32_t mods);


    std::string_view m_title;
    window_config_s m_config;

    GLFWmonitor *m_monitor_ptr = nullptr;
    GLFWwindow *m_window_ptr = nullptr;
    mutable bool m_is_resized = false;

    mutable mouse_s m_mouse;
    mutable keyboard_s m_keyboard;
};

void Window::Impl::resize_callback(GLFWwindow *window, const std::int32_t width, const std::int32_t height) noexcept {
    auto *wnd = static_cast<Window::Impl *>(glfwGetWindowUserPointer(window));
    wnd->m_config.extent.width = width;
    wnd->m_config.extent.height = height;
    wnd->m_is_resized = true;
}

void Window::Impl::keyboard_callback(GLFWwindow *window,
                                     const std::int32_t key,
                                     const std::int32_t /*scancode*/,
                                     const std::int32_t action,
                                     const std::int32_t mods) {
    const auto *wnd = static_cast<Window::Impl *>(glfwGetWindowUserPointer(window));

    if (key >= 0 && key <= GLFW_KEY_LAST) {
        if (action == GLFW_PRESS) {
            wnd->m_keyboard.keys[key] = true;
            wnd->m_keyboard.last_pressed_key_code = key;
        } else if (action == GLFW_RELEASE) {
            wnd->m_keyboard.keys[key] = false;
        }

        wnd->m_keyboard.shift_pressed = static_cast<std::uint32_t>(mods) & GLFW_MOD_SHIFT;
        wnd->m_keyboard.ctrl_pressed = static_cast<std::uint32_t>(mods) & GLFW_MOD_CONTROL;
        wnd->m_keyboard.alt_pressed = static_cast<std::uint32_t>(mods) & GLFW_MOD_ALT;
    }
}

void Window::Impl::mouse_movement_callback(GLFWwindow *window, const double xpos, const double ypos) {
    const auto *wnd = static_cast<Window::Impl *>(glfwGetWindowUserPointer(window));

    wnd->m_mouse.dx = xpos - wnd->m_mouse.last_x_position;
    wnd->m_mouse.dy = wnd->m_mouse.last_y_position - ypos;

    wnd->m_mouse.last_x_position = xpos;
    wnd->m_mouse.last_y_position = ypos;
}

void Window::Impl::mouse_button_callback(GLFWwindow *window,
                                         const std::int32_t button,
                                         const std::int32_t action,
                                         const std::int32_t /*mods*/) {
    const auto *wnd = static_cast<Window::Impl *>(glfwGetWindowUserPointer(window));

    if (button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

            if (button == GLFW_MOUSE_BUTTON_LEFT) {
                wnd->m_mouse.left_button_pressed = true;
            }
            if (button == GLFW_MOUSE_BUTTON_RIGHT) {
                wnd->m_mouse.right_button_pressed = true;
            }
        } else if (action == GLFW_RELEASE) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

            if (button == GLFW_MOUSE_BUTTON_LEFT) {
                wnd->m_mouse.left_button_pressed = false;
            }
            if (button == GLFW_MOUSE_BUTTON_RIGHT) {
                wnd->m_mouse.right_button_pressed = false;
            }
        }
    }
}

Window::Window(const app_config_s &config) : m_pimpl(std::make_unique<Impl>(config)) {}

required_instance_extensions_s Window::required_instance_extensions() const noexcept {
    return m_pimpl->required_instance_extensions();
}

bool Window::should_close() const noexcept { return m_pimpl->should_close(); }

std::string_view Window::title() const noexcept { return m_pimpl->title(); }

window_extent_s Window::extent() const noexcept { return m_pimpl->extent(); }

void Window::pool_events() const noexcept { m_pimpl->pool_events(); }

VkSurfaceKHR Window::create_surface(const VkInstance &instance) const { return m_pimpl->create_surface(instance); }

bool Window::is_resized() const noexcept { return m_pimpl->is_resized(); }

void Window::reset_resize_state() noexcept { m_pimpl->reset_resize_state(); }

bool Window::is_key_pressed(const keyboard_key_e key) const noexcept { return m_pimpl->is_key_pressed(key); }

bool Window::is_key_released(const keyboard_key_e key) const noexcept { return m_pimpl->is_key_released(key); }

const mouse_s &Window::mouse() const noexcept { return m_pimpl->mouse(); }

void Window::reset_mouse() const noexcept { return m_pimpl->reset_mouse(); }

Window::~Window() = default;

} // namespace sm::arcane