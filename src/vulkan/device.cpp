#include "device.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_to_string.hpp>

namespace sm::arcane::vulkan {

namespace {

namespace detail {

[[nodiscard]] std::uint32_t find_graphics_queue_family_index(
        const std::vector<vk::QueueFamilyProperties> &queue_family_properties) noexcept {
    // get the first index into `queue_family_properties` which supports graphics
    const auto property_it = std::find_if(
            queue_family_properties.begin(),
            queue_family_properties.end(),
            [](const vk::QueueFamilyProperties &qfp) { return qfp.queueFlags & vk::QueueFlagBits::eGraphics; });
    assert(property_it != queue_family_properties.end());

    return static_cast<std::uint32_t>(std::distance(queue_family_properties.cbegin(), property_it));
}


void log_info_about_physical_device(const vk::raii::PhysicalDevice &physical_device) {
    const auto decode_vendor_id = [](const std::uint32_t vendor_id) -> std::string {
        // below 0x10000 are the PCI vendor IDs (https://pcisig.com/membership/member-companies)
        if (vendor_id < 0x10000) {
            switch (vendor_id) {
                case 0x1022: return "Advanced Micro Devices";
                case 0x10DE: return "NVidia Corporation";
                case 0x8086: return "Intel Corporation";
                default: return std::to_string(vendor_id);
            }
        }
        // above 0x10000 should be vkVendorIDs
        return vk::to_string(static_cast<vk::VendorId>(vendor_id));
    };

    const auto picked_properties = physical_device.getProperties2().properties;

    const auto driver_version = picked_properties.driverVersion;
    const auto major_version = (driver_version >> 22) & 0x3FF;
    const auto minor_version = (driver_version >> 14) & 0xFF;
    const auto patch_version = (driver_version >> 6) & 0xFF;

    const auto vulkan_logger = spdlog::default_logger()->clone("vulkan");
    vulkan_logger->info("Picked physical device:"
                        "\n\tVendor: {}"
                        "\n\tDevice Name: {}"
                        "\n\tDevice Type: {}"
                        "\n\tDevice Driver: {}.{}.{}"
                        "\n\tVulkan API Version: {}.{}.{}",
                        decode_vendor_id(picked_properties.vendorID),
                        picked_properties.deviceName.data(),
                        vk::to_string(picked_properties.deviceType),
                        major_version,
                        minor_version,
                        patch_version,
                        VK_API_VERSION_MAJOR(picked_properties.apiVersion),
                        VK_API_VERSION_MINOR(picked_properties.apiVersion),
                        VK_API_VERSION_PATCH(picked_properties.apiVersion));
}

} // namespace detail

[[nodiscard]] std::pair<std::uint32_t, std::uint32_t> find_graphics_and_present_family_indices(
        const vk::raii::PhysicalDevice &physical_device,
        const vk::SurfaceKHR surface) {
    const auto queue_family_properties = physical_device.getQueueFamilyProperties();
    assert(queue_family_properties.size() < std::numeric_limits<std::uint32_t>::max());

    const auto graphics_index = detail::find_graphics_queue_family_index(queue_family_properties);
    if (physical_device.getSurfaceSupportKHR(graphics_index, surface)) {
        // the first `graphics_queue_family_index` does also support presents
        return {graphics_index, graphics_index};
    }

    // the `graphics_queue_family_index` doesn't support `present` -> look for another family index that supports both
    // `graphics` and `present`
    for (auto i = std::size_t{0}; i < queue_family_properties.size(); ++i) {
        if ((queue_family_properties[i].queueFlags & vk::QueueFlagBits::eGraphics) &&
            physical_device.getSurfaceSupportKHR(static_cast<std::uint32_t>(i), surface)) {
            return {static_cast<std::uint32_t>(i), static_cast<std::uint32_t>(i)};
        }
    }

    // there's nothing like a single family index that supports both `graphics` and `present` -> look for another family
    // index that supports `present`
    for (auto i = std::size_t{0}; i < queue_family_properties.size(); ++i) {
        if (physical_device.getSurfaceSupportKHR(static_cast<std::uint32_t>(i), surface)) {
            return {graphics_index, static_cast<std::uint32_t>(i)};
        }
    }

    throw std::runtime_error{"Failed to find the queues for both graphics or present"};
}


[[nodiscard]] device_queue_families_s find_queue_families(const vk::raii::Device &device,
                                                          const vk::raii::PhysicalDevice &physical_device,
                                                          const vk::SurfaceKHR surface) {
    const auto [graphics_index, present_index] = find_graphics_and_present_family_indices(physical_device, surface);
    return {.graphics = {.index = graphics_index, .queue = {device, graphics_index, 0}},
            .present = {.index = present_index, .queue = {device, present_index, 0}}};
}

[[nodiscard]] vk::raii::PhysicalDevice pick_physical_device(const vk::raii::Instance &instance) {
    const auto physical_devices = instance.enumeratePhysicalDevices();
    if (physical_devices.empty()) {
        throw std::runtime_error{"No Vulkan physical devices available"};
    }

    auto picked_device = physical_devices.front();
    detail::log_info_about_physical_device(picked_device);
    return picked_device;
}

[[nodiscard]] vk::raii::Device create_logical_device(
        // TODO: to support config for `(1) Note`. It is necessary to make a branch to select the necessary features
        /*const config_s &config*/
        const vk::raii::PhysicalDevice &physical_device) {
    auto supported_features = physical_device.getFeatures2<vk::PhysicalDeviceFeatures2,
                                                           vk::PhysicalDeviceDynamicRenderingFeaturesKHR,
                                                           vk::PhysicalDeviceSynchronization2FeaturesKHR>();

    auto &dynamic_rendering_features = supported_features.get<vk::PhysicalDeviceDynamicRenderingFeaturesKHR>();
    auto &synchronization2_features = supported_features.get<vk::PhysicalDeviceSynchronization2FeaturesKHR>();

    dynamic_rendering_features.dynamicRendering = VK_TRUE;
    synchronization2_features.synchronization2 = VK_TRUE;

    static constexpr auto device_extensions = std::array{VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                                                         VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
                                                         VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME};

    const auto queue_family_index = detail::find_graphics_queue_family_index(
            physical_device.getQueueFamilyProperties());
    auto queue_priority = 0.0f;
    const auto device_queue_info = vk::DeviceQueueCreateInfo{{}, queue_family_index, 1, &queue_priority};
    const auto device_create_info = vk::DeviceCreateInfo{
            {},
            device_queue_info,
            {},
            device_extensions,
            // (1) Note: `device_features` rarely has a feature set.
            // Only for non-regular goals. For example, `Ray Tracing`
            &supported_features.get<vk::PhysicalDeviceFeatures2>().features,
            &dynamic_rendering_features};

    return {physical_device, device_create_info};
}

} // namespace

Device::Device(const vk::raii::Instance &instance, const vk::SurfaceKHR surface)
    : m_physical_device{pick_physical_device(instance)},
      m_device{create_logical_device(m_physical_device)},
      m_queue_families{find_queue_families(m_device, m_physical_device, surface)} {}

} // namespace sm::arcane::vulkan