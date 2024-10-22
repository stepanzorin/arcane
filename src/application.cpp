#include "application.hpp"

#include <vector>
#include <string>

#include <fmt/format.h>
#include <range/v3/to_container.hpp>
#include <range/v3/view/transform.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/logger.h>

namespace sm::arcane {

Application::Application(const app_config_s &config)
    : m_logger{spdlog::default_logger()->clone("app")},
      m_window{config},
      m_instance{create_vulkan_instance(config)} {}

void Application::run() {
    while (!m_window.should_close()) {
        m_window.pool_events();
    }
}

vk::UniqueInstance Application::create_vulkan_instance(const app_config_s &config) const {
    const auto &layers = config.vulkan_config.layers;

    const auto validation_layers = layers.validation_layers |
                                   ranges::views::transform([](const std::string &str) { return str.c_str(); }) |
                                   ranges::to<std::vector<const char *>>;

#ifndef NDEBUG
    const auto validation_layers_list{fmt::format("Validation layers: \n\t{}", fmt::join(validation_layers, "\n\t"))};
    m_logger->debug("{}\n", validation_layers_list);
#endif

    auto monitor_layers = layers.monitor_layers |
                          ranges::views::transform([](const std::string &str) { return str.c_str(); }) |
                          ranges::to<std::vector<const char *>>;

#ifndef NDEBUG
    if (!true /* check_validation_layers_support() */) {
        throw std::runtime_error{"Requested validation layer is not available"};
    }
    monitor_layers.insert(monitor_layers.end(), validation_layers.begin(), validation_layers.end());
#endif

    const auto monitor_layers_list{fmt::format("Vulkan monitor layers: \n\t{}", fmt::join(monitor_layers, "\n\t"))};
    m_logger->info("{}\n", monitor_layers_list);

    auto [extension_count, extensions] = m_window.required_instance_extensions();

    const auto application_info = vk::ApplicationInfo{m_window.title().data(),
                                                      config.version.major,
                                                      config.title.c_str(),
                                                      config.version.major,
                                                      VK_API_VERSION_1_0};

    const auto create_info = vk::InstanceCreateInfo{vk::InstanceCreateFlags{},
                                                    &application_info,
                                                    static_cast<std::uint32_t>(monitor_layers.size()),
                                                    monitor_layers.data(),
                                                    extension_count,
                                                    extensions};

    return vk::createInstanceUnique(create_info);
}

Application::~Application() = default;

} // namespace sm::arcane