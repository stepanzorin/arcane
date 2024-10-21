#include "vulkan_config.hpp"

#include <string>
#include <utility>

#include <boost/json.hpp>
#include <boost/json/value.hpp>
#include <boost/json/value_to.hpp>

namespace sm::arcane {

namespace json = boost::json;

namespace {

namespace detail {

[[nodiscard]] std::vector<const char *> vector_string_to_vector_const_char(const std::vector<std::string> &strings) {
    auto vector_cstr = std::vector<const char *>{};
    vector_cstr.reserve(strings.size());
    for (const auto &str : strings) {
        vector_cstr.push_back(str.c_str());
    }
    return vector_cstr;
}
[[nodiscard]] std::vector<std::string> vector_const_char_to_vector_string(const std::vector<const char *> &cstrs) {
    auto vector_string = std::vector<std::string>{};
    vector_string.reserve(cstrs.size());
    for (const auto *cstr : cstrs) {
        vector_string.emplace_back(cstr);
    }
    return vector_string;
}

} // namespace detail

[[nodiscard]] vulkan_layers_s vulkan_layers_from_json(const json::value &layers_desc) {
    const auto validation_layers = json::value_to<std::vector<std::string>>(layers_desc.at("validation_layers"));
    const auto monitor_layers = json::value_to<std::vector<std::string>>(layers_desc.at("monitor_layers"));
    return {.validation_layers = detail::vector_string_to_vector_const_char(validation_layers),
            .monitor_layers = detail::vector_string_to_vector_const_char(monitor_layers)};
}
[[nodiscard]] json::value vulkan_layers_to_json(const vulkan_layers_s &layers) {
    auto validation_layers = json::array(layers.validation_layers.begin(), layers.validation_layers.end());
    auto monitor_layers = json::array(layers.monitor_layers.begin(), layers.monitor_layers.end());
    return {{"validation_layers", std::move(validation_layers)}, {"monitor_layers", std::move(monitor_layers)}};
}

[[nodiscard]] device_config_s device_config_from_json(const json::value &device_desc) {
    const auto extensions = json::value_to<std::vector<std::string>>(device_desc.at("extensions"));
    return {.enable_anisotropy = json::value_to<bool>(device_desc.at("enable_anisotropy")),
            .max_anisotropy = json::value_to<float>(device_desc.at("max_anisotropy")),
            .extensions = detail::vector_string_to_vector_const_char(extensions)};
}
[[nodiscard]] json::value device_config_to_json(const device_config_s &device) {
    auto extensions = json::array(device.extensions.begin(), device.extensions.end());
    return {{"enable_anisotropy", device.enable_anisotropy},
            {"max_anisotropy", device.max_anisotropy},
            {"extensions", std::move(extensions)}};
}

} // namespace

[[nodiscard]] vulkan_config_s vulkan_config_from_json(const json::value &desc) {
    const auto extensions = json::value_to<std::vector<std::string>>(desc.at("extensions"));
    return {.layers = vulkan_layers_from_json(desc.at("layers")),
            .extensions = detail::vector_string_to_vector_const_char(extensions),
            .device = device_config_from_json(desc.at("device"))};
}
[[nodiscard]] json::value vulkan_config_to_json(const vulkan_config_s &config) {
    auto extensions = json::array(detail::vector_const_char_to_vector_string(config.extensions).begin(),
                                  detail::vector_const_char_to_vector_string(config.extensions).end());
    return {{"layers", vulkan_layers_to_json(config.layers)},
            {"extensions", std::move(extensions)},
            {"device", device_config_to_json(config.device)}};
}

} // namespace sm::arcane