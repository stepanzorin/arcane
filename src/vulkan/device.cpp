#include "device.hpp"

#include <algorithm>
#include <cassert>
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
    const auto property_it = std::find_if(
            queue_family_properties.begin(),
            queue_family_properties.end(),
            [](const vk::QueueFamilyProperties &qfp) { return qfp.queueFlags & vk::QueueFlagBits::eGraphics; });
    assert(property_it != queue_family_properties.end());

    return static_cast<std::uint32_t>(std::distance(queue_family_properties.cbegin(), property_it));
}

namespace detail {

[[nodiscard]] std::pair<std::uint32_t, std::uint32_t> find_graphics_and_present_family_indices(
        const vk::raii::PhysicalDevice &physical_device,
        const vk::SurfaceKHR surface) {
    const auto queue_family_properties = physical_device.getQueueFamilyProperties();
    assert(queue_family_properties.size() < std::numeric_limits<std::uint32_t>::max());

    const auto graphics_index = find_graphics_queue_family_index(queue_family_properties);
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

} // namespace detail

[[nodiscard]] device_queue_families_s find_queue_families(const vk::raii::Device &device,
                                                          const vk::raii::PhysicalDevice &physical_device,
                                                          const vk::SurfaceKHR surface) {
    const auto [graphics_index, present_index] = detail::find_graphics_and_present_family_indices(physical_device,
                                                                                                  surface);
    return {.graphics = {.index = graphics_index, .queue = {device, graphics_index, 0}},
            .present = {.index = present_index, .queue = {device, present_index, 0}}};
}

[[nodiscard]] vk::raii::PhysicalDevice pick_physical_device(const vk::raii::Instance &instance) {
    const auto physical_devices = instance.enumeratePhysicalDevices();
    if (physical_devices.empty()) {
        throw std::runtime_error{"No Vulkan physical devices available"};
    }

    auto picked_device = physical_devices.front();

    const auto picked_properties = picked_device.getProperties2().properties;
    const auto driver_version = picked_properties.driverVersion;
    const auto major_version = (driver_version >> 22) & 0x3FF;
    const auto minor_version = (driver_version >> 14) & 0xFF;
    const auto patch_version = (driver_version >> 6) & 0xFF;

    const auto vulkan_logger = spdlog::default_logger()->clone("vulkan");
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
        // TODO: to support config for `(1) Note`. It is necessary to make a branch to select the necessary features
        /*const config_s &config*/
        const vk::raii::PhysicalDevice &physical_device) {
    static constexpr auto device_extensions = std::array{VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    const auto queue_family_index = find_graphics_queue_family_index(physical_device.getQueueFamilyProperties());
    auto queue_priority = 0.0f;
    const auto device_queue_info = vk::DeviceQueueCreateInfo{{}, queue_family_index, 1, &queue_priority};
    const auto device_create_info = vk::DeviceCreateInfo{{},
                                                         device_queue_info,
                                                         {},
                                                         device_extensions,
                                                         // (1) Note: `device_features` rarely has a feature set.
                                                         // Only for non-regular goals. For example, `Ray Tracing`
                                                         nullptr,
                                                         nullptr};

    return {physical_device, device_create_info};
}

[[nodiscard]] std::uint32_t find_memory_type(const vk::PhysicalDeviceMemoryProperties &memory_properties,
                                             std::uint32_t type_bits,
                                             const vk::MemoryPropertyFlags requirements_mask) noexcept {
    auto type_index = std::numeric_limits<std::uint32_t>::max();
    for (auto i = 0u; i < memory_properties.memoryTypeCount; ++i) {
        if ((type_bits & 1) &&
            ((memory_properties.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask)) {
            type_index = i;
            break;
        }
        type_bits >>= 1;
    }
    assert(type_index != std::numeric_limits<std::uint32_t>::max());

    return type_index;
}

[[nodiscard]] vk::raii::DeviceMemory allocate_device_memory_impl(
        const vk::raii::Device &device,
        const vk::PhysicalDeviceMemoryProperties memory_properties,
        const vk::MemoryRequirements2 requirements,
        const vk::MemoryPropertyFlags memory_property_flags) {
    const auto memory_type_index = find_memory_type(memory_properties,
                                                    requirements.memoryRequirements.memoryTypeBits,
                                                    memory_property_flags);
    return device.allocateMemory({requirements.memoryRequirements.size, memory_type_index});
}

} // namespace

Device::Device(const vk::raii::Instance &instance, const vk::SurfaceKHR surface)
    : m_physical_device{pick_physical_device(instance)},
      m_device{create_logical_device(m_physical_device)},
      m_queue_families{find_queue_families(m_device, m_physical_device, surface)} {}

vk::raii::DeviceMemory Device::allocate_buffer_device_memory(
        const vk::Buffer buffer,
        const vk::MemoryPropertyFlags memory_property_flags) const {
    assert(buffer && "vk::Buffer must be initialized for the allocation");
    assert(memory_property_flags && "Memory property flags for vk::Buffer must be initialized");
    return allocate_device_memory_impl(m_device,
                                       m_physical_device.getMemoryProperties(),
                                       m_device.getBufferMemoryRequirements2(buffer),
                                       memory_property_flags);
}

vk::raii::DeviceMemory Device::allocate_image_device_memory(const vk::Image image,
                                                            const vk::MemoryPropertyFlags memory_property_flags) const {
    assert(image && "vk::Image must be initialized for the allocation");
    assert(memory_property_flags && "Memory property flags for vk::Image must be initialized");
    return allocate_device_memory_impl(m_device,
                                       m_physical_device.getMemoryProperties(),
                                       m_device.getImageMemoryRequirements2(image),
                                       memory_property_flags);
}

} // namespace sm::arcane::vulkan