#include "debug_utils.hpp"

#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>

namespace sm::arcane::vulkan {

VKAPI_ATTR VkBool32 VKAPI_CALL
debug_utils_messenger_callback(const VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                               const VkDebugUtilsMessageTypeFlagsEXT message_types,
                               const VkDebugUtilsMessengerCallbackDataEXT *callback_data_ptr,
                               void * /* user_data_ptr */) noexcept {
#if SM_ARCANE_DEBUG_MODE
    // ReSharper disable once CppDefaultCaseNotHandledInSwitchStatement
    switch (static_cast<std::uint32_t>(callback_data_ptr->messageIdNumber)) {
        // Validation Warning: Override layer has override paths set to C:/VulkanSDK/<version>/Bin
        case 0u: [[fallthrough]];

        // Validation Warning: vkCreateInstance(): to enable extension VK_EXT_debug_utils, but this extension is
        // intended to support use by applications when debugging, and it is strongly recommended that it be
        // otherwise avoided.
        case 0x822806fa: [[fallthrough]];

        // Validation Performance Warning: Using debug builds of the validation layers *will* adversely affect
        // performance.
        case 0xe8d1a9fe: return vk::False;
    }
#endif

    const auto logger = spdlog::default_logger()->clone("vulkan_debug_messenger");
    logger->error("{}", callback_data_ptr->pMessage);

    return vk::False;
}

vk::DebugUtilsMessengerCreateInfoEXT make_debug_utils_messenger_create_info_ext() noexcept {
    return {{},
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                    vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,
            &debug_utils_messenger_callback};
}

} // namespace sm::arcane::vulkan