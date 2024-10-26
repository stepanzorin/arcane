// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include <vulkan/vk_platform.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace sm::arcane::vulkan {

[[nodiscard]] VKAPI_ATTR VkBool32 VKAPI_CALL
debug_utils_messenger_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                               VkDebugUtilsMessageTypeFlagsEXT message_types,
                               const VkDebugUtilsMessengerCallbackDataEXT *callback_data_ptr,
                               void * /* user_data_ptr */) noexcept;

[[nodiscard]] vk::DebugUtilsMessengerCreateInfoEXT make_debug_utils_messenger_create_info_ext() noexcept;

} // namespace sm::arcane::vulkan