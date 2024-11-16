// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include <chrono>
#include <cstdint>
#include <format>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "frame.hpp"
#include "vulkan/device_memory.hpp"

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

    [[nodiscard]] frame_info_s &frame_info() noexcept { return m_current_frame_info; }
    [[nodiscard]] std::uint32_t frame_index() const noexcept { return m_current_frame_info.frame_index; }
    [[nodiscard]] std::uint32_t image_index() const noexcept { return m_current_frame_info.image_index; }
    [[nodiscard]] float frame_dt() const noexcept {
        return std::chrono::duration<float>(std::chrono::steady_clock::now() - m_current_frame_info.started_time)
                .count();
    }

    [[nodiscard]] DeviceMemoryBuffer create_device_memory_buffer(
            const vk::Flags<vk::BufferUsageFlagBits> usages,
            const vk::DeviceSize size,
            const vk::DeviceSize offset = 0,
            vk::MemoryPropertyFlags memory_property_flags = vk::MemoryPropertyFlagBits::eHostVisible |
                                                            vk::MemoryPropertyFlagBits::eHostCoherent) const {
        return {m_physical_device, m_device, usages, size, offset, memory_property_flags};
    }

    [[nodiscard]] DeviceMemoryImage create_device_memory_image(const vk::Format format,
                                                               const vk::Extent2D extent,
                                                               const vk::ImageTiling tiling,
                                                               const vk::ImageUsageFlags usage,
                                                               const vk::ImageLayout initial_layout,
                                                               const vk::MemoryPropertyFlags memory_properties,
                                                               const vk::ImageAspectFlags aspect_mask) const {
        return {m_physical_device,
                m_device,
                format,
                extent,
                tiling,
                usage,
                initial_layout,
                memory_properties,
                aspect_mask};
    }

    template<typename T, typename... Args>
    void set_object_name(T object, std::format_string<Args...> fmt, Args &&...args) const {
        m_device.setDebugUtilsObjectNameEXT(
                vk::DebugUtilsObjectNameInfoEXT{object.objectType,
                                                reinterpret_cast<std::uint64_t>(typename T::NativeType(object)),
                                                std::format(fmt, std::forward<Args>(args)...).c_str()});
    }

private:
    vk::raii::PhysicalDevice m_physical_device;
    vk::raii::Device m_device;

    device_queue_families_s m_queue_families;

    frame_info_s m_current_frame_info;
};

} // namespace sm::arcane::vulkan