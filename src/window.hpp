// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include <memory>
#include <cstdint>
#include <string_view>

#include "app_config.hpp"

namespace sm::arcane {

struct required_instance_extensions_s {
    std::uint32_t count;
    const char** extensions;
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
    [[nodiscard]] bool should_close() const noexcept;
    void pool_events() const noexcept;

    ~Window();

private:
    class Impl;
    std::unique_ptr<Impl> m_pimpl;
};

} // namespace sm::arcane