// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "vulkan/device.hpp"
#include "window.hpp"

namespace sm::arcane::vulkan {

class Swapchain {
public:
    explicit Swapchain(Device &device,
                       const Window &window,
                       vk::SurfaceKHR surface,
                       Swapchain *old_swapchain_ptr = nullptr);

    [[nodiscard]] vk::SwapchainKHR handle() const noexcept { return *m_swapchain; }
    void revalue();

private:
    Device &m_device;
    const Window &m_window;
    vk::SurfaceKHR m_surface;

    vk::raii::CommandPool m_command_pool = nullptr;
    vk::raii::CommandBuffer m_command_buffer = nullptr;

    vk::Extent2D m_extent;
    Swapchain *m_old_swapchain_ptr = nullptr;
    vk::raii::SwapchainKHR m_swapchain = nullptr;

    vk::Format m_color_format{};
    std::vector<vk::Image> m_images; // TODO: to do using VMA
    std::vector<vk::raii::ImageView> m_image_views; // TODO: to do using VMA
    vk::Flags<vk::ImageUsageFlagBits> m_image_usages;

    vk::Format m_depth_format{};
    vk::raii::Image m_depth_image = nullptr; // TODO: to do using VMA
    vk::raii::ImageView m_depth_image_view = nullptr; // TODO: to do using VMA
    vk::Flags<vk::ImageUsageFlagBits> m_depth_image_usages;
};

} // namespace sm::arcane::vulkan