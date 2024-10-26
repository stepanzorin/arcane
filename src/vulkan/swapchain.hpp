// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include <memory>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "vulkan/device.hpp"
#include "window.hpp"

namespace sm::arcane::vulkan {

class Swapchain {
public:
    explicit Swapchain(const Device &device, const Window &window, const vk::raii::SurfaceKHR &surface);

    void revalue();

private:
    const Device &m_device;
    const Window &m_window;
    const vk::raii::SurfaceKHR &m_surface;

    vk::Format m_format;

    vk::raii::CommandPool m_command_pool;
    vk::raii::CommandBuffer m_command_buffer;

    vk::Extent2D m_extent;

    vk::raii::SwapchainKHR m_swapchain = nullptr;

    std::vector<vk::Image> m_images;

    std::unique_ptr<vk::raii::SwapchainKHR> m_old_swapchain_khr;
};

} // namespace sm::arcane::vulkan