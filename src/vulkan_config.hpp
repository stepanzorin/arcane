// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include <vector>

#include <boost/describe/class.hpp>
#include <boost/json/fwd.hpp>

namespace sm::arcane {

struct vulkan_layers_s {
    std::vector<const char *> validation_layers;
    std::vector<const char *> monitor_layers;

    BOOST_DESCRIBE_STRUCT(vulkan_layers_s, (), (validation_layers, monitor_layers))
};

struct device_config_s {
    bool enable_anisotropy;
    float max_anisotropy;
    std::vector<const char *> extensions;

    BOOST_DESCRIBE_STRUCT(device_config_s, (), (enable_anisotropy, max_anisotropy, extensions))
};

struct vulkan_config_s {
    vulkan_layers_s layers;
    std::vector<const char *> extensions;
    device_config_s device;

    BOOST_DESCRIBE_STRUCT(vulkan_config_s, (), (layers, extensions, device))
};

[[nodiscard]] vulkan_config_s vulkan_config_from_json(const boost::json::value & /* desc */);
[[nodiscard]] boost::json::value vulkan_config_to_json(const vulkan_config_s & /* config */);

} // namespace sm::arcane