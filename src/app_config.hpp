// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

#include <boost/describe/class.hpp>
#include <boost/describe/enum.hpp>
#include <boost/json/fwd.hpp>

#include "vulkan/config.hpp"
#include "window_config.hpp"

namespace sm::arcane {

namespace detail {

struct version_levels_s {
    std::uint8_t major = SM_ARCANE_PROJECT_VERSION_MAJOR;
    std::uint8_t minor = SM_ARCANE_PROJECT_VERSION_MINOR;
    std::uint8_t patch = SM_ARCANE_PROJECT_VERSION_PATCH;

    [[nodiscard]] bool operator==(const version_levels_s &) const noexcept = default;

    BOOST_DESCRIBE_STRUCT(version_levels_s, (), (major, minor, patch))
};

} // namespace detail

struct app_config_s {
    std::filesystem::path config_path; // is populated once when the application starts

    std::string title = SM_ARCANE_PROJECT_NAME;
    detail::version_levels_s version;
    window_config_s window_config;
    vulkan::config_s vulkan;

    BOOST_DESCRIBE_STRUCT(app_config_s, (), (title, version, window_config, window_config))
};

[[nodiscard]] app_config_s app_config_from_json(const std::filesystem::path & /* config_path */);
void app_config_to_json(const app_config_s & /* updated_config */);

} // namespace sm::arcane