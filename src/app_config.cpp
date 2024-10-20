#include "app_config.hpp"

#include <fstream>
#include <stdexcept>
#include <string_view>

#include <boost/json.hpp>
#include <boost/json/array.hpp>
#include <boost/json/value.hpp>
#include <boost/json/value_to.hpp>
#include <fmt/format.h>

#include "util/pretty_json.hpp"

namespace sm::arcane {

namespace detail {

void validate_version(const version_levels_s config_project_version) {
    static constexpr auto cmake_project_version = version_levels_s{SM_ARCANE_PROJECT_VERSION_MAJOR,
                                                                   SM_ARCANE_PROJECT_VERSION_MINOR,
                                                                   SM_ARCANE_PROJECT_VERSION_PATCH};

    if (config_project_version != cmake_project_version) {
        throw std::runtime_error(fmt::format("Config project version [{}, {}, {}] doesn't match with version indicated "
                                             "in the root CMakeLists.txt [{}, {}, {}]",
                                             config_project_version.major,
                                             config_project_version.minor,
                                             config_project_version.patch,
                                             cmake_project_version.major,
                                             cmake_project_version.minor,
                                             cmake_project_version.patch));
    }
}

} // namespace detail

namespace json = boost::json;

namespace {

inline constexpr auto g_config_path = SM_ARCANE_APP_CONFIG_PATH;

[[nodiscard]] json::value parse_app_config() {
    auto file = std::ifstream{g_config_path};
    if (!file.is_open()) {
        throw std::runtime_error{"Filed to open the app config file for the reading"};
    }

    const auto json_content = std::string{(std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()};
    return json::parse(json_content);
}

[[nodiscard]] std::string title_from_json(const json::value &desc) noexcept { return desc.as_string().c_str(); }
[[nodiscard]] json::value title_to_json(const std::string_view title) { return title.data(); }

[[nodiscard]] detail::version_levels_s collect_levels(const json::array &version_levels) noexcept {
    const auto major = json::value_to<std::uint8_t>(version_levels[0]);
    const auto minor = json::value_to<std::uint8_t>(version_levels[1]);
    const auto patch = json::value_to<std::uint8_t>(version_levels[2]);
    return {.major = major, .minor = minor, .patch = patch};
}
[[nodiscard]] detail::version_levels_s version_from_json(const json::value &desc) {
    const auto version_levels = collect_levels(desc.as_array());
    detail::validate_version(version_levels);
    return version_levels;
}
[[nodiscard]] json::value version_to_json(const detail::version_levels_s version) {
    return {version.major, version.minor, version.patch};
}

} // namespace

app_config_s app_config_from_json() {
    const auto &json_desc = parse_app_config();
    const auto &app_desc = json_desc.at("app");
    return {.title = title_from_json(app_desc.at("title")),
            .version = version_from_json(app_desc.at("version")),
            .window_config = window_config_from_json(app_desc.at("window")),
            .vulkan_config = vulkan_config_from_json(app_desc.at("vulkan"))};
}
void app_config_to_json(const app_config_s &config) {
    auto file = std::ofstream{g_config_path};
    if (!file.is_open()) {
        throw std::runtime_error{"Failed to open the app config file for the writing"};
    }

    const auto json_value = json::value{{"app",
                                         {{"title", title_to_json(config.title)},
                                          {"version", version_to_json(config.version)},
                                          {"window", window_config_to_json(config.window_config)},
                                          {"vulkan", vulkan_config_to_json(config.vulkan_config)}}}};

    util::write_pretty_json(file, json_value);
}

} // namespace sm::arcane