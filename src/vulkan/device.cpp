#include "device.hpp"

#include <algorithm>
#include <cstddef>
#include <limits>
#include <ranges>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <utility>
#include <vector>

#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_to_string.hpp>

namespace sm::arcane::vulkan {

namespace {

[[nodiscard]] std::uint32_t find_graphics_queue_family_index(
        const std::vector<vk::QueueFamilyProperties> &queue_family_properties) noexcept {
    // get the first index into `queue_family_properties` which supports graphics
    const auto graphics_queue_family_property = std::find_if(
            queue_family_properties.begin(),
            queue_family_properties.end(),
            [](const vk::QueueFamilyProperties &qfp) { return qfp.queueFlags & vk::QueueFlagBits::eGraphics; });
    assert(graphics_queue_family_property != queue_family_properties.end());

    return static_cast<std::uint32_t>(std::distance(queue_family_properties.begin(), graphics_queue_family_property));
}

[[nodiscard]] queue_family_indices_s find_graphics_and_present_queue_family_index(
        const vk::raii::PhysicalDevice &physical_device,
        const vk::raii::SurfaceKHR &surface) {
    const auto queue_family_properties = physical_device.getQueueFamilyProperties();
    assert(queue_family_properties.size() < (std::numeric_limits<std::uint32_t>::max)());

    const auto graphics_index = find_graphics_queue_family_index(queue_family_properties);
    if (physical_device.getSurfaceSupportKHR(graphics_index, surface)) {
        // the first `graphics_queue_family_index` does also support presents
        return {.graphics_index = graphics_index, .present_index = graphics_index};
    }

    // the `graphics_queue_family_index` doesn't support `present` -> look for another family index that supports both
    // `graphics` and `present`
    for (auto i = std::size_t{0}; i < queue_family_properties.size(); ++i) {
        if ((queue_family_properties[i].queueFlags & vk::QueueFlagBits::eGraphics) &&
            physical_device.getSurfaceSupportKHR(static_cast<std::uint32_t>(i), surface)) {
            return {.graphics_index = static_cast<std::uint32_t>(i), .present_index = static_cast<std::uint32_t>(i)};
        }
    }

    // there's nothing like a single family index that supports both `graphics` and `present` -> look for another family
    // index that supports `present`
    for (auto i = std::size_t{0}; i < queue_family_properties.size(); ++i) {
        if (physical_device.getSurfaceSupportKHR(static_cast<std::uint32_t>(i), surface)) {
            return {.graphics_index = graphics_index, .present_index = static_cast<std::uint32_t>(i)};
        }
    }

    throw std::runtime_error("Could not find queues for both graphics or present -> terminating");
}

[[nodiscard]] vk::raii::PhysicalDevice pick_physical_device(const vk::raii::Instance &instance) {
    const auto physical_devices = instance.enumeratePhysicalDevices();
    if (physical_devices.empty()) {
        throw std::runtime_error("No Vulkan physical devices available.");
    }

    const auto vulkan_logger = spdlog::default_logger()->clone("vulkan");

    auto picked_device = physical_devices.front();

    const auto picked_properties = picked_device.getProperties2().properties;
    const auto driver_version = picked_properties.driverVersion;
    const auto major_version = (driver_version >> 22) & 0x3FF;
    const auto minor_version = (driver_version >> 14) & 0xFF;
    const auto patch_version = (driver_version >> 6) & 0xFF;
    vulkan_logger->info("Picked physical device:"
                        "\n\tDevice Name: {}"
                        "\n\tDevice Type: {}"
                        "\n\tDevice Driver: {}.{}.{}"
                        "\n\tVulkan API Version: {}.{}.{}",
                        picked_properties.deviceName.data(),
                        vk::to_string(picked_properties.deviceType),
                        major_version,
                        minor_version,
                        patch_version,
                        VK_VERSION_MAJOR(picked_properties.apiVersion),
                        VK_VERSION_MINOR(picked_properties.apiVersion),
                        VK_VERSION_PATCH(picked_properties.apiVersion));

    return picked_device;
}

[[nodiscard]] vk::raii::Device create_logical_device(
        const vk::raii::PhysicalDevice &physical_device,
        const vk::PhysicalDeviceFeatures *physical_device_features_ptr = nullptr) {
    static constexpr auto device_extensions = std::array{VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    const auto queue_family_index = find_graphics_queue_family_index(physical_device.getQueueFamilyProperties());
    auto queue_priority = 0.0f;
    const auto device_queue_info = vk::DeviceQueueCreateInfo{{}, queue_family_index, 1, &queue_priority};
    const auto device_create_info = vk::DeviceCreateInfo{{},
                                                         device_queue_info,
                                                         {},
                                                         device_extensions,
                                                         physical_device_features_ptr,
                                                         nullptr};

    return {physical_device, device_create_info};
}

} // namespace

Device::Device(const vk::raii::Instance &instance, const vk::raii::SurfaceKHR &surface)
    : m_physical_device{pick_physical_device(instance)},
      m_device{create_logical_device(m_physical_device)},
      m_queue_family_indices{find_graphics_and_present_queue_family_index(m_physical_device, surface)} {}

} // namespace sm::arcane::vulkan