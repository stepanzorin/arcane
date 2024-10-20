#include "window.hpp"

#include <cstdint>
#include <stdexcept>
#include <string_view>
#include <utility>

#define GLFW_INCLUDE_NONE
#include <glfw/glfw3.h>
#include <spdlog/spdlog.h>

namespace sm::arcane {

namespace {

#if WIN32
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

#if WIN32
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

    ~GLFWInitializer() { glfwTerminate(); }
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
    }

    [[nodiscard]] bool should_close() const noexcept { return glfwWindowShouldClose(m_window_ptr); }

    static void pool_events() noexcept { return glfwPollEvents(); }

    ~Impl() { glfwDestroyWindow(m_window_ptr); }

private:
    std::string_view m_title;
    window_config_s m_config;

    GLFWmonitor *m_monitor_ptr = nullptr;
    GLFWwindow *m_window_ptr = nullptr;
};

Window::Window(const app_config_s &config) : m_pimpl(std::make_unique<Impl>(config)) {}

bool Window::should_close() const noexcept { return m_pimpl->should_close(); }

void Window::pool_events() const noexcept { m_pimpl->pool_events(); }

Window::~Window() = default;

} // namespace sm::arcane