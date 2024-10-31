// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace sm::arcane::vulkan {

struct DeviceMemoryBuffer {
private:
    vk::PhysicalDevice physical_device;
    vk::Device device;

public:
    DeviceMemoryBuffer(const vk::raii::PhysicalDevice &physical_device,
                       const vk::raii::Device &device,
                       vk::BufferUsageFlags usages,
                       vk::DeviceSize size,
                       vk::DeviceSize offset,
                       vk::MemoryPropertyFlags property_flags = vk::MemoryPropertyFlagBits::eHostVisible |
                                                                vk::MemoryPropertyFlagBits::eHostCoherent);

    explicit(false) DeviceMemoryBuffer(std::nullptr_t) {}

    vk::raii::Buffer buffer = nullptr;
    vk::raii::DeviceMemory device_memory = nullptr;
#if SM_ARCANE_DEBUG_MODE
    vk::DeviceSize size{};
    vk::BufferUsageFlags usages;
    vk::MemoryPropertyFlags property_flags;
#endif
};

struct DeviceMemoryImage {
private:
    vk::PhysicalDevice m_physical_device;
    vk::Device m_device;

public:
    DeviceMemoryImage(const vk::raii::PhysicalDevice &physical_device,
                      const vk::raii::Device &device,
                      vk::Format format,
                      vk::Extent2D extent,
                      vk::ImageTiling tiling,
                      vk::ImageUsageFlags usage,
                      vk::ImageLayout initial_layout,
                      vk::MemoryPropertyFlags memory_properties,
                      vk::ImageAspectFlags aspect_mask);

    explicit(false) DeviceMemoryImage(std::nullptr_t) {}

    // the DeviceMemory should be destroyed before the Image it is bound to; to get that order with the standard
    // destructor of the ImageData, the order of DeviceMemory and Image here matters
    vk::Format m_format{};
    vk::raii::Image m_image = nullptr;
    vk::raii::DeviceMemory m_device_memory = nullptr;
    vk::raii::ImageView m_image_view = nullptr;
};

} // namespace sm::arcane::vulkan