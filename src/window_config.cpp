#include "window_config.hpp"

#include <cstdint>
#include <stdexcept>
#include <utility>

#include <boost/json.hpp>
#include <boost/json/value.hpp>
#include <boost/json/value_to.hpp>

namespace sm::arcane {

namespace json = boost::json;

namespace {

namespace detail {

[[nodiscard]] window_mode_e window_mode_string_to_enum(const std::string_view mode_str) {
    if (mode_str == "windowed") {
        return window_mode_e::windowed;
    }

    if (mode_str == "maximized") {
        return window_mode_e::maximized;
    }

    if (mode_str == "fullscreen") {
        return window_mode_e::fullscreen;
    }

    throw std::invalid_argument{"Failed to serialize 'windows_mode_e` from JSON. Invalid argument"};
}
[[nodiscard]] std::string window_mode_enum_to_string(const window_mode_e mode) noexcept {
    switch (mode) {
        case window_mode_e::windowed: return "windowed";
        case window_mode_e::maximized: return "maximized";
        case window_mode_e::fullscreen: return "fullscreen";
    }
    std::unreachable();
}

} // namespace detail

[[nodiscard]] window_extent_s window_extent_from_json(const json::value &extent_desc) {
    return {.width = json::value_to<std::int32_t>(extent_desc.at("width")),
            .height = json::value_to<std::int32_t>(extent_desc.at("height"))};
}
[[nodiscard]] json::value window_extent_to_json(const window_extent_s &extent) {
    return {{"width", extent.width}, {"height", extent.height}};
}

[[nodiscard]] window_position_s window_position_from_json(const json::value &position_desc) {
    return {.x = json::value_to<std::int32_t>(position_desc.at("x")),
            .y = json::value_to<std::int32_t>(position_desc.at("y"))};
}
[[nodiscard]] json::value window_position_to_json(const window_position_s &position) {
    return {{"x", position.x}, {"y", position.y}};
}

[[nodiscard]] window_mode_e window_mode_from_json(const json::value &desc) {
    const auto &mode_str = desc.as_string();
    return detail::window_mode_string_to_enum(mode_str);
}
[[nodiscard]] json::value window_mode_to_json(const window_mode_e mode) {
    return detail::window_mode_enum_to_string(mode).c_str();
}

} // namespace

[[nodiscard]] window_config_s window_config_from_json(const json::value &desc) {
    const auto &window_desc = desc.as_object();
    return {.extent = window_extent_from_json(window_desc.at("extent")),
            .position = window_position_from_json(window_desc.at("position")),
            .mode = window_mode_from_json(window_desc.at("mode"))};
}
[[nodiscard]] json::value window_config_to_json(const window_config_s &config) {
    return {{"extent", window_extent_to_json(config.extent)},
            {"position", window_position_to_json(config.position)},
            {"mode", window_mode_to_json(config.mode)}};
}

} // namespace sm::arcane