#include "instance.hpp"

#include <cstring>
#include <memory>
#include <string>

#include <fmt/format.h>
#include <range/v3/algorithm/any_of.hpp>
#ifdef SM_ARCANE_DEBUG_MODE
    #include <range/v3/algorithm/for_each.hpp>
    #include <range/v3/algorithm/none_of.hpp>
#endif
#include <range/v3/to_container.hpp>
#include <range/v3/view/transform.hpp>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>

#include "debug_utils.hpp"

namespace sm::arcane::vulkan {

namespace {

[[nodiscard]] std::vector<char const *> gather_extensions(const std::shared_ptr<spdlog::logger> &vulkan_logger,
                                                          const std::vector<std::string> &instance_extensions
#if SM_ARCANE_DEBUG_MODE
                                                          ,
                                                          std::vector<vk::ExtensionProperties> extension_properties
#endif
) {
#if SM_ARCANE_DEBUG_MODE
    ranges::for_each(instance_extensions, [&](const std::string &ext) {
        assert(ranges::any_of(extension_properties,
                              [&](const vk::ExtensionProperties &ep) { return ext == ep.extensionName; }));
    });
#endif

    auto enabled_extensions = instance_extensions |
                              ranges::views::transform([](const std::string &ext) { return ext.data(); }) |
                              ranges::to<std::vector<const char *>>();

#if SM_ARCANE_DEBUG_MODE
    if (ranges::none_of(
                instance_extensions,
                [](const std::string_view extension) { return extension == VK_EXT_DEBUG_UTILS_EXTENSION_NAME; }) &&
        ranges::any_of(extension_properties, [](const vk::ExtensionProperties &ep) {
            return (std::strcmp(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, ep.extensionName) == 0);
        })) {
        enabled_extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
#endif

    const auto monitor_layers_list = fmt::format(
            "Vulkan instance extensions:{}",
            fmt::join(enabled_extensions | ranges::views::transform([](const std::string &layer) {
                          return fmt::format("\n\t{}", layer);
                      }),
                      ""));
    vulkan_logger->info("{}", monitor_layers_list);

    return enabled_extensions;
}

[[nodiscard]] std::vector<const char *> gather_layers(const std::shared_ptr<spdlog::logger> &vulkan_logger,
                                                      const bool enable_validation_layers,
                                                      const std::vector<std::string> &monitor_layers
#if SM_ARCANE_DEBUG_MODE
                                                      ,
                                                      std::vector<vk::LayerProperties> layer_properties
#endif
) {

#ifdef SM_ARCANE_DEBUG_MODE
    ranges::for_each(monitor_layers, [&](const std::string &layer) {
        assert(ranges::any_of(layer_properties, [&](const vk::LayerProperties &lp) { return layer == lp.layerName; }));
    });
#endif

    auto enabled_layers = monitor_layers |
                          ranges::views::transform([](const std::string &layer) { return layer.data(); }) |
                          ranges::to<std::vector<const char *>>();

#if SM_ARCANE_DEBUG_MODE
    if (enable_validation_layers) {
        enabled_layers.emplace_back("VK_LAYER_KHRONOS_validation");
    }
#endif

    const auto monitor_layers_list = fmt::format(
            "Vulkan monitor layers:{}",
            fmt::join(enabled_layers | ranges::views::transform(
                                               [](const std::string &layer) { return fmt::format("\n\t{}", layer); }),
                      ""));
    vulkan_logger->info("{}", monitor_layers_list);

    return enabled_layers;
}

[[nodiscard]] vk::raii::Instance create_vulkan_instance(const app_config_s &config) {
    const auto vulkan_logger = spdlog::default_logger()->clone("vulkan");

    const auto context = vk::raii::Context{};

    static const auto monitor_layers = std::vector<std::string>{"VK_LAYER_LUNARG_monitor"};

    static const auto instance_extensions = std::vector<std::string>{
            VK_KHR_SURFACE_EXTENSION_NAME,
#if defined(VK_USE_PLATFORM_WIN32_KHR)
            VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif defined(VK_USE_PLATFORM_XCB_KHR)
            VK_KHR_XCB_SURFACE_EXTENSION_NAME,
#elif SM_ARCANE_OPERATING_SYSTEM_MACOS
            VK_EXT_METAL_SURFACE_EXTENSION_NAME,
#endif
    };

    const auto enabled_layers = gather_layers(vulkan_logger,
                                              config.vulkan.enable_validation_layers,
                                              monitor_layers
#if SM_ARCANE_DEBUG_MODE
                                              ,
                                              context.enumerateInstanceLayerProperties()
#endif
    );

    const auto enabled_extensions = gather_extensions(vulkan_logger,
                                                      instance_extensions
#if SM_ARCANE_DEBUG_MODE
                                                      ,
                                                      context.enumerateInstanceExtensionProperties()
#endif
    );

    const auto application_info = vk::ApplicationInfo{config.title.c_str(),
                                                      config.version.major,
                                                      config.title.c_str(),
                                                      config.version.major,
                                                      VK_API_VERSION_1_3};

#if SM_ARCANE_DEBUG_MODE
    const auto severity_flags = vk::DebugUtilsMessageSeverityFlagsEXT{
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError};

    const auto message_type_flags = vk::DebugUtilsMessageTypeFlagsEXT{
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation};

    const auto instance_create_info = vk::StructureChain<vk::InstanceCreateInfo, vk::DebugUtilsMessengerCreateInfoEXT>{
            {{}, &application_info, enabled_layers, enabled_extensions},
            {{}, severity_flags, message_type_flags, &debug_utils_messenger_callback}};
#else
    auto instance_create_info = vk::StructureChain<vk::InstanceCreateInfo>{
            {{}, &application_info, enabled_layers, enabled_extensions}};
#endif

    return {context, instance_create_info.get<vk::InstanceCreateInfo>()};
}
} // namespace

Instance::Instance(const app_config_s &config)
    : m_instance{create_vulkan_instance(config)}
#if SM_ARCANE_DEBUG_MODE
      ,
      // in debug mode, additionally should to use the `vulkan::debug_utils_messenger_callback` in instance creation
      m_debug_messenger{m_instance.createDebugUtilsMessengerEXT(make_debug_utils_messenger_create_info_ext())}
#endif
{
}

} // namespace sm::arcane::vulkan