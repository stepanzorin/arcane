// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "vulkan/device.hpp"
#include "vulkan/device_memory.hpp"
#include "window.hpp"

namespace sm::arcane::vulkan {

class Swapchain {
public:
    Swapchain(Device &device, const Window &window, vk::SurfaceKHR surface, Swapchain *old_swapchain_ptr = nullptr);

    [[nodiscard]] vk::SwapchainKHR handle() const noexcept { return *m_swapchain; }
    [[nodiscard]] const vk::raii::SwapchainKHR &get() const noexcept { return m_swapchain; }
    [[nodiscard]] const vk::raii::CommandPool &command_pool() const noexcept { return m_command_pool; }
    [[nodiscard]] const vk::raii::CommandBuffers &command_buffers() const noexcept { return m_command_buffers; }
    [[nodiscard]] const std::vector<vk::Image> &color_images() const noexcept { return m_color_images; }
    [[nodiscard]] vk::Format color_format() const noexcept { return m_color_format; }
    [[nodiscard]] vk::Format depth_format() const noexcept { return m_depth_dm_image.format; }
    [[nodiscard]] const std::vector<vk::raii::ImageView> &color_image_views() const noexcept { return m_image_views; }
    [[nodiscard]] const DeviceMemoryImage &depth_image() const noexcept { return m_depth_dm_image; }
    [[nodiscard]] vk::SurfaceKHR surface() const noexcept { return m_surface; }
    [[nodiscard]] vk::Extent2D extent() const noexcept { return m_extent; }
    [[nodiscard]] float aspect_ratio() const noexcept;

    void acquire_next_image(const vk::Semaphore &wait_semaphore, vk::Fence fence = nullptr) const;

    void revalue();

private:
    Device &m_device;
    const Window &m_window;
    vk::SurfaceKHR m_surface;

    vk::raii::CommandPool m_command_pool = nullptr;
    vk::raii::CommandBuffers m_command_buffers = nullptr;

    vk::Extent2D m_extent;
    Swapchain *m_old_swapchain_ptr = nullptr;
    vk::raii::SwapchainKHR m_swapchain = nullptr;

    vk::Format m_color_format{};
    std::vector<vk::Image> m_color_images; // TODO: to do using VMA
    std::vector<vk::raii::ImageView> m_image_views; // TODO: to do using VMA
    vk::Flags<vk::ImageUsageFlagBits> m_image_usages;

    DeviceMemoryImage m_depth_dm_image = nullptr; // TODO: to do using VMA
};

} // namespace sm::arcane::vulkan