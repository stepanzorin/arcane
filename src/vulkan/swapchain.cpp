#include "swapchain.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <utility>

#include <range/v3/algorithm/find_if.hpp>
#include <spdlog/spdlog.h>

namespace sm::arcane::vulkan {

namespace {

[[nodiscard]] vk::PresentModeKHR pick_present_mode(const std::vector<vk::PresentModeKHR> &present_modes) noexcept {
    auto picked_mode = vk::PresentModeKHR::eFifo; // The FIFO present mode is guaranteed by the spec to be supported

    for (const auto &present_mode : present_modes) {
        if (present_mode == vk::PresentModeKHR::eMailbox) {
            picked_mode = present_mode;
            break;
        }

        if (present_mode == vk::PresentModeKHR::eImmediate) {
            picked_mode = present_mode;
        }
    }

    return picked_mode;
}

[[nodiscard]] vk::SurfaceFormatKHR pick_surface_format(const std::vector<vk::SurfaceFormatKHR> &formats) noexcept {
    assert(!formats.empty());

    auto picked_format = formats.front();

    if (formats.size() == 1) {
        if (formats[0].format == vk::Format::eUndefined) {
            picked_format.format = vk::Format::eB8G8R8A8Unorm;
            picked_format.colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
        }
    } else {
        // request several formats, the first found will be used
        static constexpr auto requested_formats = std::array{vk::Format::eB8G8R8A8Unorm,
                                                             vk::Format::eR8G8B8A8Unorm,
                                                             vk::Format::eB8G8R8Unorm,
                                                             vk::Format::eR8G8B8Unorm};

        constexpr auto requested_color_space = vk::ColorSpaceKHR::eSrgbNonlinear;
        for (const auto requested_format : requested_formats) {
            const auto it = ranges::find_if(formats, [&](const vk::SurfaceFormatKHR &f) {
                return (f.format == requested_format) && (f.colorSpace == requested_color_space);
            });

            if (it != formats.cend()) {
                picked_format = *it;
                break;
            }
        }
    }

    assert(picked_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear);
    return picked_format;
}

[[nodiscard]] vk::Format pick_depth_format(const vk::raii::PhysicalDevice &physical_device) {
    constexpr auto candidates = std::array{vk::Format::eD32Sfloat,
                                           vk::Format::eD32SfloatS8Uint,
                                           vk::Format::eD24UnormS8Uint};

    for (const auto format : candidates) {
        const auto properties = physical_device.getFormatProperties(format);
        if (properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
            return format;
        }
    }

    throw std::runtime_error{"Failed to find supported depth stencil format"};
}

} // namespace

Swapchain::Swapchain(Device &device,
                     const Window &window,
                     const vk::raii::SurfaceKHR &surface,
                     Swapchain *old_swapchain_ptr /* = nullptr */)
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
                                         .front())},
      m_old_swapchain_sptr{old_swapchain_ptr} {
    revalue();
}

void Swapchain::revalue() {
    const auto &physical_device = m_device.physical_device();

    const auto surface_capabilities = physical_device.getSurfaceCapabilitiesKHR(m_surface);

    const auto surface_format = pick_surface_format(physical_device.getSurfaceFormatsKHR(*m_surface));
    m_color_format = surface_format.format;

    m_extent = [&, this] -> vk::Extent2D {
        if (surface_capabilities.currentExtent.width != (std::numeric_limits<std::uint32_t>::max)()) {
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

    m_image_usages = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc;

    const auto queue_family_indices = std::array{m_device.queue_family_indices().graphics_index,
                                                 m_device.queue_family_indices().present_index};

    const auto swapchain_info = vk::SwapchainCreateInfoKHR{
            vk::SwapchainCreateFlagsKHR{},
            m_surface,
            std::clamp(3u, surface_capabilities.minImageCount, surface_capabilities.maxImageCount),
            m_color_format,
            surface_format.colorSpace,
            m_extent,
            1,
            m_image_usages,
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
            pick_present_mode(physical_device.getSurfacePresentModesKHR(*m_surface)),
            true,
            m_old_swapchain_sptr ? m_old_swapchain_sptr->handle() : nullptr};

    m_old_swapchain_sptr = nullptr;
    m_swapchain = vk::raii::SwapchainKHR{m_device.device(), swapchain_info};
    m_images = m_swapchain.getImages();

    const auto depth_stencil_format = pick_depth_format(physical_device);
    m_depth_format = depth_stencil_format;

    static const auto vulkan_logger = spdlog::default_logger()->clone("vulkan");
    vulkan_logger->set_level(spdlog::level::trace);
    vulkan_logger->trace("Swapchain is recreated:"
                         "\n\tExtent: {}x{}"
                         "\n\tColor format: {}"
                         "\n\tColor space: {}"
                         "\n\tDepth stencil format: {}",
                         m_extent.width,
                         m_extent.height,
                         vk::to_string(surface_format.format),
                         vk::to_string(surface_format.colorSpace),
                         vk::to_string(depth_stencil_format));
}

} // namespace sm::arcane::vulkan