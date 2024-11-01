// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace sm::arcane::vulkan {

struct DeviceMemoryBuffer {
private:
    template<typename T>
    void copy_to_memory(const T *const src_ptr, const std::size_t count, const vk::DeviceSize stride = sizeof(T)) {
        assert(sizeof(T) <= stride);
        auto *dst_ptr = static_cast<std::uint8_t *>(device_memory.mapMemory(0, count * stride, {}));
        if (stride == sizeof(T)) {
            std::memcpy(dst_ptr, src_ptr, count * sizeof(T));
        } else {
            for (auto i = std::size_t{0}; i < count; ++i) {
                std::memcpy(dst_ptr, &src_ptr[i], sizeof(T));
                dst_ptr += stride;
            }
        }
        device_memory.unmapMemory();
    }

    template<typename T>
    void copy_to_memory(const T &data) {
        copy_to_memory<T>(&data, 1);
    }

    vk::PhysicalDevice physical_device = nullptr;
    vk::Device device = nullptr;

public:
    vk::raii::Buffer buffer = nullptr;
    vk::raii::DeviceMemory device_memory = nullptr;
#if !defined(_NDEBUG)
    vk::DeviceSize size{};
    vk::BufferUsageFlags usages{};
    vk::MemoryPropertyFlags property_flags{};
#endif

    DeviceMemoryBuffer(const vk::raii::PhysicalDevice &physical_device,
                       const vk::raii::Device &device,
                       vk::BufferUsageFlags usages,
                       vk::DeviceSize size,
                       vk::DeviceSize offset = 0,
                       vk::MemoryPropertyFlags property_flags = vk::MemoryPropertyFlagBits::eHostVisible |
                                                                vk::MemoryPropertyFlagBits::eHostCoherent);

    explicit(false) DeviceMemoryBuffer(std::nullptr_t) {}

    template<typename T>
    void upload(const T &data) const {
        assert((property_flags & vk::MemoryPropertyFlagBits::eHostCoherent) &&
               (property_flags & vk::MemoryPropertyFlagBits::eHostVisible));
        assert(sizeof(T) <= size);

        auto *data_ptr = device_memory.mapMemory(0, sizeof(T), {});
        std::memcpy(data_ptr, &data, sizeof(T));
        device_memory.unmapMemory();
    }

    template<typename T>
    void upload(const std::span<T> data, const std::size_t stride = 0) const {
        assert(property_flags & vk::MemoryPropertyFlagBits::eHostVisible);

        const auto element_size = stride ? stride : sizeof(T);
        assert(sizeof(T) <= element_size);

        copy_to_memory(data.data(), data.size(), element_size);
    }
};

struct DeviceMemoryImage {
private:
    vk::PhysicalDevice physical_device = nullptr;
    vk::Device device = nullptr;

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
    vk::Format format{};
    vk::raii::Image image = nullptr;
    vk::raii::DeviceMemory device_memory = nullptr;
    vk::raii::ImageView image_view = nullptr;
};

} // namespace sm::arcane::vulkan