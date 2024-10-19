// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include <cstdint>

#include <boost/describe/class.hpp>
#include <boost/describe/enum.hpp>
#include <boost/json/fwd.hpp>

namespace sm::arcane {

enum class window_mode_e { //
    windowed,
    maximized,
    fullscreen
};
BOOST_DESCRIBE_ENUM(window_mode_e, windowed, maximized, fullscreen)

struct window_extent_s {
    std::int32_t width = 0;
    std::int32_t height = 0;

    BOOST_DESCRIBE_STRUCT(window_extent_s, (), (width, height))
};

struct window_position_s {
    std::int32_t x = 0;
    std::int32_t y = 0;

    BOOST_DESCRIBE_STRUCT(window_position_s, (), (x, y))
};

struct window_config_s {
    window_extent_s extent;
    window_position_s position;
    window_mode_e mode = window_mode_e::windowed;

    BOOST_DESCRIBE_STRUCT(window_config_s, (), (extent, position, mode))
};

[[nodiscard]] window_config_s window_config_from_json(const boost::json::value & /* desc */);
[[nodiscard]] boost::json::value window_config_to_json(const window_config_s & /* config */);

} // namespace sm::arcane