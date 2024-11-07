// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include <memory>
#include <vector>

#include <spdlog/logger.h>
#include <vulkan/vulkan_raii.hpp>

#include "app_config.hpp"
#include "renderer.hpp"
#include "vulkan/device.hpp"
#include "vulkan/instance.hpp"
#include "vulkan/swapchain.hpp"
#include "window.hpp"

namespace sm::arcane {

struct vulkan_layers_s {
    std::vector<const char *> monitors;
    std::vector<const char *> validations;
};

class Application {
public:
    explicit Application() = delete;

    explicit Application(const app_config_s &config);
    Application(const Application &) = delete;
    Application &operator=(const Application &) = delete;
    Application(Application &&) noexcept = delete;
    Application &operator=(Application &&) noexcept = delete;

    void run();

    [[nodiscard]] std::unique_ptr<vulkan::Swapchain> create_swapchain();
    void recreate_swapchain();

    ~Application() = default;

private:
    std::shared_ptr<spdlog::logger> m_logger;

    Window m_window;

    vulkan::Instance m_instance;

    vk::raii::SurfaceKHR m_surface;
    vulkan::Device m_device;
    std::unique_ptr<vulkan::Swapchain> m_swapchain_uptr;

    Renderer m_renderer;
};

} // namespace sm::arcane
