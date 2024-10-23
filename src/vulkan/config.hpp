// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include <string>
#include <vector>

#include <boost/describe/class.hpp>
#include <boost/json/fwd.hpp>

namespace sm::arcane::vulkan {

// Note
/*

The reason for using `std::vector<std::string>` instead of `std::vector<const char*>` is that working with `boost::json`
would require additional steps. `boost::json` does not support direct conversion from `const char*` because JSON strings
are objects, not raw character pointers.

TODO: In the future, when possible, `std::vector<std::string>` should be replaced with `std::vector<const char*>`.

*/

struct device_config_s {
    bool enable_anisotropy;
    float max_anisotropy;

    BOOST_DESCRIBE_STRUCT(device_config_s, (), (enable_anisotropy, max_anisotropy))
};

struct config_s {
    bool enable_validation_layers;
    device_config_s device;

    BOOST_DESCRIBE_STRUCT(config_s, (), (enable_validation_layers, device))
};

[[nodiscard]] config_s config_from_json(const boost::json::value & /* desc */);
[[nodiscard]] boost::json::value config_to_json(const config_s & /* config */);

} // namespace sm::arcane::vulkan