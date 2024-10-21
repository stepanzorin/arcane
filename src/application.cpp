#include "application.hpp"

#include <spdlog/spdlog.h>

namespace sm::arcane {

namespace {

// The function needs due to Boost::JSON woks only with `const char*` and `Vulkan API` works only with `const char *`
[[nodiscard]] std::vector<const char *> vector_string_to_vector_const_char(const std::vector<std::string> &strings) {
    auto vector_cstr = std::vector<const char *>{};
    vector_cstr.reserve(strings.size());
    for (const auto &str : strings) {
        vector_cstr.push_back(str.c_str());
    }
    return vector_cstr;
}

} // namespace

Application::Application(const app_config_s &config)
    : m_logger{spdlog::default_logger()->clone("app")}, //
      m_window{config},
      m_instance{create_vulkan_instance(config)} {}

void Application::run() {
    while (!m_window.should_close()) {
        m_window.pool_events();
    }
}

vk::UniqueInstance Application::create_vulkan_instance(const app_config_s &config) const {
    const auto &layers = config.vulkan_config.layers;
    const auto validation_layers = vector_string_to_vector_const_char(layers.validation_layers);

#ifndef NDEBUG
    const auto validation_layers_list{fmt::format("Validation layers: \n\t{}", fmt::join(validation_layers, "\n\t"))};
    m_logger->debug("{}\n", validation_layers_list);
#endif

    auto monitor_layers = vector_string_to_vector_const_char(layers.monitor_layers);

#ifndef NDEBUG
    if (!true /* check_validation_layers_support() */) {
        throw std::runtime_error{"Requested validation layer is not available"};
    }
    monitor_layers.insert(monitor_layers.end(), validation_layers.begin(), validation_layers.end());
#endif

    const auto monitor_layers_list{fmt::format("Vulkan monitor layers: \n\t{}", fmt::join(monitor_layers, "\n\t"))};
    m_logger->info("{}\n", monitor_layers_list);

    auto [extension_count, extensions] = m_window.get_required_instance_extensions();

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