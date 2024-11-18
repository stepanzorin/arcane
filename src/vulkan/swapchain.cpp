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
[[nodiscard]] vk::PresentModeKHR pick_present_mode(const vk::PhysicalDevice physical_device,
                                                   const vk::SurfaceKHR surface) noexcept {
    auto picked_mode = vk::PresentModeKHR::eFifo; // The FIFO present mode is guaranteed by the spec to be supported

    const auto present_modes = physical_device.getSurfacePresentModesKHR(surface);
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

[[nodiscard]] vk::SurfaceFormatKHR pick_surface_format(const vk::PhysicalDevice physical_device,
                                                       const vk::SurfaceKHR surface) noexcept {
    const auto formats = physical_device.getSurfaceFormatsKHR(surface);
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

        static constexpr auto requested_color_space = vk::ColorSpaceKHR::eSrgbNonlinear;
        for (const auto requested_format : requested_formats) {
            const auto found_format_it = ranges::find_if(formats, [&](const vk::SurfaceFormatKHR &f) {
                return f.format == requested_format && f.colorSpace == requested_color_space;
            });

            if (found_format_it != formats.cend()) {
                picked_format = *found_format_it;
                break;
            }
        }
    }
    assert(picked_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear);

    return picked_format;
}

[[nodiscard]] vk::Format pick_depth_format(const vk::PhysicalDevice physical_device) {
    static constexpr auto candidates = std::array{vk::Format::eD32Sfloat,
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

[[nodiscard]] vk::ImageTiling pick_depth_tiling_format(const vk::PhysicalDevice physical_device,
                                                       const vk::Format depth_format) {
    const auto depth_format_properties = physical_device.getFormatProperties(depth_format);

    if (depth_format_properties.linearTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
        return vk::ImageTiling::eLinear;
    }

    if (depth_format_properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
        return vk::ImageTiling::eOptimal;
    }

    throw std::runtime_error{
            fmt::format("DepthStencilAttachment is not supported for {} depth format", vk::to_string(depth_format))};
}

} // namespace

Swapchain::Swapchain(Device &device,
                     const Window &window,
                     const vk::SurfaceKHR surface,
                     Swapchain *old_swapchain_ptr /* = nullptr */)
    : m_device{device},
      m_window{window},
      m_surface{surface},
      // create a `CommandPool` to allocate a `CommandBuffer` from
      m_command_pool{
              vk::raii::CommandPool{device.device(),
                                    vk::CommandPoolCreateInfo{{vk::CommandPoolCreateFlagBits::eResetCommandBuffer},
                                                              m_device.queue_families().graphics.index}}},
      m_old_swapchain_ptr{old_swapchain_ptr} {
    revalue();
}

float Swapchain::aspect_ratio() const noexcept {
    return static_cast<float>(m_extent.width) / static_cast<float>(m_extent.height);
}

void Swapchain::acquire_next_image(const vk::Semaphore &wait_semaphore, const vk::Fence fence) const {
    auto [result,
          image_index] = m_swapchain.acquireNextImage(std::numeric_limits<std::uint64_t>::max(), wait_semaphore, fence);
    assert(image_index < m_color_images.size());
    if (result == vk::Result::eErrorOutOfDateKHR) {
        return;
    }
    if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
        throw std::runtime_error("Failed to acquire swapchain image");
    }

    m_device.frame_info().image_index = image_index;
}

void Swapchain::revalue() {
    const auto &physical_device = m_device.physical_device();

    const auto surface_capabilities = physical_device.getSurfaceCapabilitiesKHR(m_surface);

    const auto surface_format = pick_surface_format(physical_device, m_surface);

    static const auto vulkan_logger = spdlog::default_logger()->clone("vulkan");
    vulkan_logger->info("Surface:"
                        "\n\tColor format: {}"
                        "\n\tColor space: {}",
                        vk::to_string(surface_format.format),
                        vk::to_string(surface_format.colorSpace));

    m_color_format = surface_format.format;

    m_extent = [&, this] -> vk::Extent2D {
        if (surface_capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max()) {
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

    const auto queue_family_indices = std::array{m_device.queue_families().graphics.index,
                                                 m_device.queue_families().graphics.index};

    const auto swapchain_info = vk::SwapchainCreateInfoKHR{
            vk::SwapchainCreateFlagsKHR{},
            m_surface,
            std::clamp(3u, surface_capabilities.minImageCount, surface_capabilities.maxImageCount),
            m_color_format,
            surface_format.colorSpace,
            m_extent,
            1,
            m_image_usages,
            m_device.queue_families().are_different() ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive,
            m_device.queue_families().are_different() ? static_cast<std::uint32_t>(queue_family_indices.size()) : 0,
            m_device.queue_families().are_different() ? queue_family_indices.data() : nullptr,
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
            pick_present_mode(*physical_device, m_surface),
            true,
            m_old_swapchain_ptr ? m_old_swapchain_ptr->handle() : nullptr};

    m_swapchain = m_device.device().createSwapchainKHR(swapchain_info);

    m_color_images = m_swapchain.getImages();
    m_image_views.clear();
    m_image_views.reserve(m_color_images.size());
    {
        auto image_view_create_info = vk::ImageViewCreateInfo{{},
                                                              {},
                                                              vk::ImageViewType::e2D,
                                                              m_color_format,
                                                              {},
                                                              {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};
        for (const auto image : m_color_images) {
            image_view_create_info.image = image;
            m_image_views.emplace_back(m_device.device(), image_view_create_info);
        }
    }

    m_depth_dm_image = m_device.create_device_memory_image(
            pick_depth_format(physical_device),
            m_extent,
            pick_depth_tiling_format(*physical_device, pick_depth_format(physical_device)),
            vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled,
            vk::ImageLayout::eUndefined,
            vk::MemoryPropertyFlagBits::eDeviceLocal,
            vk::ImageAspectFlagBits::eDepth);

    // allocate main `CommandBuffer`s from the `CommandPool` for each swap chain image and reuse for rendering
    m_command_buffers = std::move(
            vk::raii::CommandBuffers{m_device.device(),
                                     vk::CommandBufferAllocateInfo{m_command_pool,
                                                                   vk::CommandBufferLevel::ePrimary,
                                                                   static_cast<std::uint32_t>(m_color_images.size())}}),

    vulkan_logger->set_level(spdlog::level::trace);
    vulkan_logger->trace("Swapchain is recreated:"
                         "\n\tExtent: {}x{}"
                         "\n\tColor format: {}"
                         "\n\tDepth stencil format: {}",
                         m_extent.width,
                         m_extent.height,
                         vk::to_string(m_color_format),
                         vk::to_string(m_depth_dm_image.format));
}

} // namespace sm::arcane::vulkan