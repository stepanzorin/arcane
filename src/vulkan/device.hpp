// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include <cstdint>
#include <format>

#include <vulkan/vulkan_raii.hpp>

namespace sm::arcane::vulkan {

struct queue_family_indices_s {
    std::uint32_t graphics_index = 0;
    std::uint32_t present_index = 0;

    [[nodiscard]] bool are_different() const noexcept { return graphics_index != present_index; }
};

class Device {
public:
    Device() = delete;
    explicit Device(const vk::raii::Instance &instance, const vk::raii::SurfaceKHR &surface);

    [[nodiscard]] const vk::raii::PhysicalDevice &physical_device() const noexcept { return m_physical_device; }
    [[nodiscard]] const vk::raii::Device &device() const noexcept { return m_device; }
    [[nodiscard]] queue_family_indices_s queue_family_indices() const noexcept { return m_queue_family_indices; }

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

    queue_family_indices_s m_queue_family_indices;
};

} // namespace sm::arcane::vulkan