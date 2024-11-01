#include "device_memory.hpp"

#include <cassert>
#include <cstdint>
#include <limits>
#include <ranges>

namespace sm::arcane::vulkan {

namespace {

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

DeviceMemoryBuffer::DeviceMemoryBuffer(
        const vk::raii::PhysicalDevice &physical_device,
        const vk::raii::Device &device,
        const vk::BufferUsageFlags usages,
        const vk::DeviceSize size,
        const vk::DeviceSize offset,
        const vk::MemoryPropertyFlags property_flags /* = vk::MemoryPropertyFlagBits::eHostVisible |
                                                          vk::MemoryPropertyFlagBits::eHostCoherent */)
    : physical_device{*physical_device},
      device{*device},
      buffer{device, {{}, size, usages}},
      device_memory{[&] {
          assert(*buffer && "vk::raii::Buffer must be initialized for the allocation");
          assert(property_flags && "Memory property flags for vk::raii::Buffer must be initialized");
          auto device_memory = allocate_device_memory_impl(device,
                                             physical_device.getMemoryProperties(),
                                             device.getBufferMemoryRequirements2(*buffer),
                                             property_flags);
          buffer.bindMemory(device_memory, offset);
          return device_memory;
      }()}
#if SM_ARCANE_DEBUG_MODE
      ,
      size{size},
      usages{usages},
      property_flags{property_flags}
#endif
{
}

DeviceMemoryImage::DeviceMemoryImage(const vk::raii::PhysicalDevice &physical_device,
                                     const vk::raii::Device &device,
                                     const vk::Format format,
                                     const vk::Extent2D extent,
                                     const vk::ImageTiling tiling,
                                     const vk::ImageUsageFlags usage,
                                     const vk::ImageLayout initial_layout,
                                     const vk::MemoryPropertyFlags memory_properties,
                                     const vk::ImageAspectFlags aspect_mask)
    : physical_device{*physical_device},
      device{*device},
      format{format},
      image{device,
            {{},
             vk::ImageType::e2D,
             format,
             vk::Extent3D{extent, 1},
             1,
             1,
             vk::SampleCountFlagBits::e1,
             tiling,
             usage,
             vk::SharingMode::eExclusive,
             {},
             nullptr,
             initial_layout}},
      device_memory{[&] {
          assert(*image && "vk::raii::Image must be initialized before the allocation");
          assert(memory_properties && "Memory property flags for vk::raii::Image must be initialized");
          auto device_memory = allocate_device_memory_impl(device,
                                                           physical_device.getMemoryProperties(),
                                                           device.getImageMemoryRequirements2(*image),
                                                           memory_properties);
          image.bindMemory(device_memory, 0);
          return device_memory;
      }()},
      image_view{device, {{}, image, vk::ImageViewType::e2D, format, {}, {aspect_mask, 0, 1, 0, 1}}} {}

} // namespace sm::arcane::vulkan