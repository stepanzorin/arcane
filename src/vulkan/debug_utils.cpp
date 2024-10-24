#include "debug_utils.hpp"

#include <cstdint>

#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>

#include "os.h"

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
        case 0u:
        [[fallthrough]]

        // Validation Warning: vkCreateInstance(): to enable extension VK_EXT_debug_utils, but this extension is
        // intended to support use by applications when debugging, and it is strongly recommended that it be
        // otherwise avoided.
        case 0x822806fa:
        [[fallthrough]]

        // Validation Performance Warning: Using debug builds of the validation layers *will* adversely affect
        // performance.
        case 0xe8d1a9fe:

            return vk::False;
    }
#endif

    const auto logger = spdlog::default_logger()->clone("vulkan_debug_messenger");

    // Map severity to spdlog levels
    if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        logger->error("{}: {}",
                      vk::to_string(static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(message_severity)),
                      vk::to_string(static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(message_types)));
    } else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        logger->warn("{}: {}",
                     vk::to_string(static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(message_severity)),
                     vk::to_string(static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(message_types)));
    } else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        logger->info("{}: {}",
                     vk::to_string(static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(message_severity)),
                     vk::to_string(static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(message_types)));
    } else {
        logger->debug("{}: {}",
                      vk::to_string(static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(message_severity)),
                      vk::to_string(static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(message_types)));
    }

    // Output detailed information
    logger->info("\tmessageIDName   = <{}>", callback_data_ptr->pMessageIdName);
    logger->info("\tmessageIdNumber = {}", callback_data_ptr->messageIdNumber);
    logger->info("\tmessage         = <{}>", callback_data_ptr->pMessage);

    // Queue Labels
    if (callback_data_ptr->queueLabelCount > 0) {
        logger->info("\tQueue Labels:");
        for (auto i = 0u; i < callback_data_ptr->queueLabelCount; ++i) {
            logger->info("\t\tlabelName = <{}>", callback_data_ptr->pQueueLabels[i].pLabelName);
        }
    }

    // CommandBuffer Labels
    if (callback_data_ptr->cmdBufLabelCount > 0) {
        logger->info("\tCommandBuffer Labels:");
        for (auto i = 0u; i < callback_data_ptr->cmdBufLabelCount; ++i) {
            logger->info("\t\tlabelName = <{}>", callback_data_ptr->pCmdBufLabels[i].pLabelName);
        }
    }

    // Objects
    if (callback_data_ptr->objectCount > 0) {
        logger->info("\tObjects:");
        for (auto i = 0u; i < callback_data_ptr->objectCount; ++i) {
            logger->info("\t\tObject {}:", i);
            logger->info("\t\t\tobjectType   = {}",
                         vk::to_string(static_cast<vk::ObjectType>(callback_data_ptr->pObjects[i].objectType)));
            logger->info("\t\t\tobjectHandle = {}", callback_data_ptr->pObjects[i].objectHandle);
            if (callback_data_ptr->pObjects[i].pObjectName) {
                logger->info("\t\t\tobjectName   = <{}>", callback_data_ptr->pObjects[i].pObjectName);
            }
        }
    }

    return vk::False;
}

vk::DebugUtilsMessengerCreateInfoEXT makeDebugUtilsMessengerCreateInfoEXT() noexcept {
    return {{},
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                    vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,
            &debug_utils_messenger_callback};
}

} // namespace sm::arcane::vulkan