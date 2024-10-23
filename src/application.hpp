// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include <memory>
#include <vector>

#include <spdlog/spdlog.h>
#include <vulkan/vulkan.hpp>

#include "app_config.hpp"
#include "window.hpp"

namespace sm::arcane {


class Application {
public:
    explicit Application() = delete;

    explicit Application(const app_config_s &config);
    Application(const Application &) = delete;
    Application &operator=(const Application &) = delete;
    Application(Application &&) noexcept = delete;
    Application &operator=(Application &&) noexcept = delete;

    void run();
    [[nodiscard]] vk::UniqueInstance create_vulkan_instance(const app_config_s &config);

    ~Application();

private:
    void init_vulkan_layers_and_extensions(const app_config_s &config);
    std::shared_ptr<spdlog::logger> m_logger;

    Window m_window;

    struct vulkan_layers_and_extensions_s {
        struct vulkan_layers_s {
            std::vector<const char *> monitors;
            std::vector<const char *> validations;
        };
        vulkan_layers_s vulkan_layers;
        std::vector<const char *> device_extensions;
    };
    vulkan_layers_and_extensions_s m_vulkan_layers_and_extensions;

    vk::UniqueInstance m_instance;
};

} // namespace sm::arcane
