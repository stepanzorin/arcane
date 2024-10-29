// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include <cstdint>
#include <format>

#include <vulkan/vulkan_raii.hpp>

namespace sm::arcane::vulkan {

struct device_queue_families_s {
    struct family_s {
        std::uint32_t index;
        vk::raii::Queue queue;
    };

    family_s graphics;
    family_s present;

    [[nodiscard]] bool are_different() const noexcept { return graphics.index != present.index; }
};

class Device {
public:
    Device() = delete;
    explicit Device(const vk::raii::Instance &instance, vk::SurfaceKHR surface);

    [[nodiscard]] const vk::raii::PhysicalDevice &physical_device() const noexcept { return m_physical_device; }
    [[nodiscard]] const vk::raii::Device &device() const noexcept { return m_device; }
    [[nodiscard]] device_queue_families_s queue_families() const noexcept { return m_queue_families; }

    [[nodiscard]] vk::raii::DeviceMemory allocate_buffer_device_memory(
            vk::Buffer buffer,
            vk::MemoryPropertyFlags memory_property_flags = vk::MemoryPropertyFlagBits::eHostVisible |
                                                            vk::MemoryPropertyFlagBits::eHostCoherent) const;

    [[nodiscard]] vk::raii::DeviceMemory allocate_image_device_memory(
            vk::Image image,
            vk::MemoryPropertyFlags memory_property_flags) const;

    template<typename T, typename... Args>
    void set_object_name(T object, std::format_string<Args...> fmt, Args &&...args) const {
        m_device.setDebugUtilsObjectNameEXT(vk::DebugUtilsObjectNameInfoEXT{
                object.objectType,
                reinterpret_cast<std::uint64_t>(typename T::NativeType(object)),
                std::format(fmt, std::forward<Args>(args)...).c_str(),
        });
    }

private:
    vk::raii::PhysicalDevice m_physical_device;
    vk::raii::Device m_device;

    device_queue_families_s m_queue_families;
};

} // namespace sm::arcane::vulkan