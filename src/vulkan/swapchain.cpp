#include "swapchain.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <utility>

namespace sm::arcane::vulkan {

Swapchain::Swapchain(const Device &device, const Window &window, const vk::raii::SurfaceKHR &surface)
    : m_device{device},
      m_window{window},
      m_surface{surface},
      // create a `CommandPool` to allocate a `CommandBuffer` from
      m_command_pool{
              vk::raii::CommandPool{device.device(),
                                    vk::CommandPoolCreateInfo{{}, device.queue_family_indices().graphics_index}}},
      // allocate a `CommandBuffer` from the `CommandPool`
      m_command_buffer{std::move(vk::raii::CommandBuffers{
              device.device(),
              vk::CommandBufferAllocateInfo{m_command_pool, vk::CommandBufferLevel::ePrimary, 1}}
                                         .front())} {
    revalue();
}

void Swapchain::revalue() {
    m_format = [this] -> vk::Format {
        const auto formats = m_device.physical_device().getSurfaceFormatsKHR(m_surface);
        assert(!formats.empty());
        return formats[0].format == vk::Format::eUndefined ? vk::Format::eB8G8R8A8Unorm : formats[0].format;
    }();

    const auto surface_capabilities = m_device.physical_device().getSurfaceCapabilitiesKHR(m_surface);

    m_extent = [&, this] -> vk::Extent2D {
        if (surface_capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)()) {
            // If the surface size is defined, the swap chain size must match
            return surface_capabilities.currentExtent;
        }

        // If the surface size is undefined, the size is set to the size of the images requested.
        return {std::clamp(static_cast<std::uint32_t>(m_window.extent().width),
                           surface_capabilities.minImageExtent.width,
                           surface_capabilities.maxImageExtent.width),
                std::clamp(static_cast<std::uint32_t>(m_window.extent().height),
                           surface_capabilities.minImageExtent.height,
                           surface_capabilities.maxImageExtent.height)};
    }();

    // The FIFO present mode is guaranteed by the spec to be supported
    constexpr auto swapchain_present_mode = vk::PresentModeKHR::eFifo;

    const auto queue_family_indices = std::array{m_device.queue_family_indices().graphics_index,
                                                 m_device.queue_family_indices().present_index};

    const auto swapchain_info = vk::SwapchainCreateInfoKHR{
            vk::SwapchainCreateFlagsKHR{},
            m_surface,
            std::clamp(3u, surface_capabilities.minImageCount, surface_capabilities.maxImageCount),
            m_format,
            vk::ColorSpaceKHR::eSrgbNonlinear,
            m_extent,
            1,
            vk::ImageUsageFlagBits::eColorAttachment,
            m_device.queue_family_indices().are_different() ? vk::SharingMode::eConcurrent
                                                            : vk::SharingMode::eExclusive,
            m_device.queue_family_indices().are_different() ? static_cast<std::uint32_t>(queue_family_indices.size())
                                                            : 0,
            m_device.queue_family_indices().are_different() ? queue_family_indices.data() : nullptr,
            vk::SurfaceTransformFlagBitsKHR{
                    (surface_capabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity)
                            ? vk::SurfaceTransformFlagBitsKHR::eIdentity
                            : surface_capabilities.currentTransform},
            vk::CompositeAlphaFlagBitsKHR{
                    (surface_capabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePreMultiplied)
                            ? vk::CompositeAlphaFlagBitsKHR::ePreMultiplied
                    : (surface_capabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePostMultiplied)
                            ? vk::CompositeAlphaFlagBitsKHR::ePostMultiplied
                    : (surface_capabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::eInherit)
                            ? vk::CompositeAlphaFlagBitsKHR::eInherit
                            : vk::CompositeAlphaFlagBitsKHR::eOpaque},
            swapchain_present_mode,
            true,
            nullptr};

    m_swapchain = vk::raii::SwapchainKHR(m_device.device(), swapchain_info);

    m_images = m_swapchain.getImages();
}

} // namespace sm::arcane::vulkan