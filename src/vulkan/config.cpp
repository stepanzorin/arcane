#include "config.hpp"

#include <boost/json.hpp>
#include <boost/json/value.hpp>
#include <boost/json/value_to.hpp>

namespace sm::arcane::vulkan {

namespace json = boost::json;

namespace {

[[nodiscard]] device_config_s device_config_from_json(const json::value &device_desc) {
    return {.enable_anisotropy = json::value_to<bool>(device_desc.at("enable_anisotropy")),
            .max_anisotropy = json::value_to<float>(device_desc.at("max_anisotropy"))};
}
[[nodiscard]] json::value device_config_to_json(const device_config_s &device) {
    return {{"enable_anisotropy", device.enable_anisotropy}, {"max_anisotropy", device.max_anisotropy}};
}

} // namespace

[[nodiscard]] config_s config_from_json(const json::value &desc) {
    return {.enable_validation_layers = json::value_to<bool>(desc.at("enable_validation_layers")),
            .device = device_config_from_json(desc.at("device"))};
}
[[nodiscard]] json::value config_to_json(const config_s &config) {
    return {{"enable_validation_layers", config.enable_validation_layers},
            {"device", device_config_to_json(config.device)}};
}

} // namespace sm::arcane::vulkan