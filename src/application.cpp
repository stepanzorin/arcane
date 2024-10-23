#include "application.hpp"

#include <string>
#include <vector>

#include <fmt/format.h>
#include <range/v3/to_container.hpp>
#include <range/v3/view/transform.hpp>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>

#include "os.h"

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

vk::UniqueInstance Application::create_vulkan_instance(const app_config_s &config) {
    init_vulkan_layers_and_extensions(config);
    auto &[layers, device_extensions] = m_vulkan_layers_and_extensions;
    auto [window_extension_count, window_extensions] = m_window.required_instance_extensions();

    const auto application_info = vk::ApplicationInfo{m_window.title().data(),
                                                      config.version.major,
                                                      config.title.c_str(),
                                                      config.version.major,
                                                      VK_API_VERSION_1_0};

    vk::InstanceCreateInfo create_info{vk::InstanceCreateFlags{},
                                       &application_info,
                                       static_cast<std::uint32_t>(layers.validations.size()),
                                       layers.monitors.data(),
                                       window_extension_count,
                                       window_extensions};

    return vk::createInstanceUnique(create_info);
}

Application::~Application() = default;

void Application::init_vulkan_layers_and_extensions(const app_config_s &config) {
    auto &[layers, device_extensions] = m_vulkan_layers_and_extensions;

    layers.monitors.emplace_back("VK_LAYER_LUNARG_monitor");

    if (!layers.monitors.empty()) {
        m_logger->info("Monitor layers:\n\t{}", fmt::join(layers.monitors, "\n\t"));
    }

#ifdef SM_ARCANE_DEBUG_MODE
    if (config.vulkan.enable_validation_layers) {
        layers.validations.emplace_back("VK_LAYER_KHRONOS_validation");
    }

    if (!layers.validations.empty()) {
        m_logger->info("Validation layers:\n\t{}", fmt::join(layers.validations, "\n\t"));
    }

    if (false /* !check_validation_layers_support() */) {
        throw std::runtime_error{"Requested validation layer is not available"};
    }
    layers.monitors.insert(layers.monitors.end(), layers.validations.begin(), layers.validations.end());

#endif

    device_extensions.emplace_back(vk::KHRSwapchainExtensionName);
#if SM_ARCANE_OPERATING_SYSTEM_WINDOWS
    device_extensions.emplace_back("VK_KHR_win32_surface");
#elif SM_ARCANE_OPERATING_SYSTEM_LINUX
    device_extensions.emplace_back("VK_KHR_xcb_surface");
#elif SM_ARCANE_OPERATING_SYSTEM_MACOS
    device_extensions.emplace_back("VK_EXT_metal_surface");
#endif

    // device_extensions.emplace_back(vk::KHRAccelerationStructureExtensionName);
    // device_extensions.emplace_back(vk::KHRRayTracingPipelineExtensionName);
    // device_extensions.emplace_back(vk::EXTDescriptorIndexingExtensionName);
    // device_extensions.emplace_back(vk::KHRMaintenance3ExtensionName);
    // device_extensions.emplace_back(vk::KHRDeferredHostOperationsExtensionName);
    // device_extensions.emplace_back(vk::KHRRayQueryExtensionName);
    // device_extensions.emplace_back(vk::KHRBufferDeviceAddressExtensionName);
    // device_extensions.emplace_back(vk::KHRSpirv14ExtensionName);
    // device_extensions.emplace_back(vk::KHRShaderFloatControlsExtensionName);

    if (!device_extensions.empty()) {
        m_logger->info("Device extensions:\n\t{}", fmt::join(device_extensions, "\n\t"));
    }
}

} // namespace sm::arcane