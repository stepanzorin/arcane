// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include <utility>

#include "app_config.hpp"
#include "window.hpp"

namespace sm::arcane {

class Application {
public:
    explicit Application() = delete;
    explicit Application(app_config_s config) : m_app_config{std::move(config)}, m_window{m_app_config} {}
    Application(const Application &) = delete;
    Application &operator=(const Application &) = delete;
    Application(Application &&) noexcept = delete;
    Application &operator=(Application &&) noexcept = delete;

    void run();

    ~Application() = default;

private:
    app_config_s m_app_config;

    Window m_window;
};

} // namespace sm::arcane
