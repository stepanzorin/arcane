#include "vulkan_config.hpp"

#include <boost/json.hpp>
#include <boost/json/value.hpp>
#include <boost/json/value_to.hpp>

namespace sm::arcane {

namespace json = boost::json;

namespace {

[[nodiscard]] vulkan_layers_s vulkan_layers_from_json(const json::value &layers_desc) {
    return {.validation_layers = json::value_to<std::vector<std::string>>(layers_desc.at("validation_layers")),
            .monitor_layers = json::value_to<std::vector<std::string>>(layers_desc.at("monitor_layers"))};
}

[[nodiscard]] json::value vulkan_layers_to_json(const vulkan_layers_s &layers) {
    auto validation_layers = json::array(layers.validation_layers.begin(), layers.validation_layers.end());
    auto monitor_layers = json::array(layers.monitor_layers.begin(), layers.monitor_layers.end());
    return {{"validation_layers", std::move(validation_layers)}, {"monitor_layers", std::move(monitor_layers)}};
}

[[nodiscard]] device_config_s device_config_from_json(const json::value &device_desc) {
    return {.enable_anisotropy = json::value_to<bool>(device_desc.at("enable_anisotropy")),
            .max_anisotropy = json::value_to<float>(device_desc.at("max_anisotropy")),
            .extensions = json::value_to<std::vector<std::string>>(device_desc.at("extensions"))};
}

[[nodiscard]] json::value device_config_to_json(const device_config_s &device) {
    auto extensions = json::array(device.extensions.begin(), device.extensions.end());
    return {{"enable_anisotropy", device.enable_anisotropy},
            {"max_anisotropy", device.max_anisotropy},
            {"extensions", std::move(extensions)}};
}

} // namespace

[[nodiscard]] vulkan_config_s vulkan_config_from_json(const json::value &desc) {
    return {.layers = vulkan_layers_from_json(desc.at("layers")),
            .extensions = json::value_to<std::vector<std::string>>(desc.at("extensions")),
            .device = device_config_from_json(desc.at("device"))};
}
[[nodiscard]] json::value vulkan_config_to_json(const vulkan_config_s &config) {
    auto extensions = json::array(config.extensions.begin(), config.extensions.end());
    return {{"layers", vulkan_layers_to_json(config.layers)},
            {"extensions", std::move(extensions)},
            {"device", device_config_to_json(config.device)}};
}


} // namespace sm::arcane