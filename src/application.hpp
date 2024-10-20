// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include <memory>

#include <spdlog/spdlog.h>

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

    ~Application();

private:
    std::shared_ptr<spdlog::logger> m_logger;
    Window m_window;
};

} // namespace sm::arcane
