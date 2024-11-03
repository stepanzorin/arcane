// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include <cassert>
#include <utility>
#include <vector>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace sm::arcane::vulkan {

namespace detail { // TODO: move to a separate place

[[nodiscard]] inline vk::AccessFlags get_access_flags(const vk::ImageLayout layout) noexcept {
    switch (layout) {
        case vk::ImageLayout::eUndefined: [[fallthrough]];
        case vk::ImageLayout::ePresentSrcKHR: return {};
        case vk::ImageLayout::ePreinitialized: return vk::AccessFlagBits::eHostWrite;
        case vk::ImageLayout::eColorAttachmentOptimal:
            return vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
        case vk::ImageLayout::eDepthAttachmentOptimal:
            return vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        case vk::ImageLayout::eFragmentShadingRateAttachmentOptimalKHR:
            return vk::AccessFlagBits::eFragmentShadingRateAttachmentReadKHR;
        case vk::ImageLayout::eShaderReadOnlyOptimal:
            return vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eInputAttachmentRead;
        case vk::ImageLayout::eTransferSrcOptimal: return vk::AccessFlagBits::eTransferRead;
        case vk::ImageLayout::eTransferDstOptimal: return vk::AccessFlagBits::eTransferWrite;
        case vk::ImageLayout::eGeneral:
            assert(false &&
                   "Don't know how to get a meaningful vk::AccessFlags for vk::ImageLayout::eGeneral! Don't use it!");
            return {};
        default: assert(false); return {};
    }
}

[[nodiscard]] inline vk::PipelineStageFlags get_pipeline_stage_flags(const vk::ImageLayout layout) noexcept {
    switch (layout) {
        case vk::ImageLayout::eUndefined: return vk::PipelineStageFlagBits::eTopOfPipe;
        case vk::ImageLayout::ePreinitialized: return vk::PipelineStageFlagBits::eHost;
        case vk::ImageLayout::eTransferDstOptimal: [[fallthrough]];
        case vk::ImageLayout::eTransferSrcOptimal: return vk::PipelineStageFlagBits::eTransfer;
        case vk::ImageLayout::eColorAttachmentOptimal: return vk::PipelineStageFlagBits::eColorAttachmentOutput;
        case vk::ImageLayout::eDepthAttachmentOptimal:
            return vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests;
        case vk::ImageLayout::eFragmentShadingRateAttachmentOptimalKHR:
            return vk::PipelineStageFlagBits::eFragmentShadingRateAttachmentKHR;
        case vk::ImageLayout::eShaderReadOnlyOptimal:
            return vk::PipelineStageFlagBits::eVertexShader | vk::PipelineStageFlagBits::eFragmentShader;
        case vk::ImageLayout::ePresentSrcKHR: return vk::PipelineStageFlagBits::eBottomOfPipe;
        case vk::ImageLayout::eGeneral:
            assert(false && "Don't know how to get a meaningful vk::PipelineStageFlags for vk::ImageLayout::eGeneral! "
                            "Don't use it!");
            return {};
        default: assert(false); return {};
    }
}

} // namespace detail

inline void image_layout_transition(const vk::CommandBuffer command_buffer,
                                    const vk::Image image,
                                    const vk::PipelineStageFlags src_stage_mask,
                                    const vk::PipelineStageFlags dst_stage_mask,
                                    const vk::AccessFlags src_access_mask,
                                    const vk::AccessFlags dst_access_mask,
                                    const vk::ImageLayout old_layout,
                                    const vk::ImageLayout new_layout,
                                    const vk::ImageSubresourceRange &subresource_range) noexcept {
    const auto image_memory_barrier = vk::ImageMemoryBarrier{/* .srcAccessMask = */ src_access_mask,
                                                             /* .dstAccessMask = */ dst_access_mask,
                                                             /* .oldLayout = */ old_layout,
                                                             /* .newLayout = */ new_layout,
                                                             /* .srcQueueFamilyIndex = */ vk::QueueFamilyIgnored,
                                                             /* .dstQueueFamilyIndex = */ vk::QueueFamilyIgnored,
                                                             /* .image = */ image,
                                                             /* .subresourceRange = */ subresource_range};

    command_buffer.pipelineBarrier(/* .srcStageMask = */ src_stage_mask,
                                   /* .dstStageMask = */ dst_stage_mask,
                                   /* .dependencyFlags = */ {},
                                   /* .memoryBarriers = */ nullptr,
                                   /* .bufferMemoryBarriers = */ nullptr,
                                   /* .imageMemoryBarriers = */ image_memory_barrier);
}

inline void image_layout_transition(const vk::CommandBuffer command_buffer,
                                    const vk::Image image,
                                    const vk::ImageLayout old_layout,
                                    const vk::ImageLayout new_layout,
                                    const vk::ImageSubresourceRange &subresource_range) noexcept {
    const auto src_stage_mask = vk::PipelineStageFlags{detail::get_pipeline_stage_flags(old_layout)};
    const auto dst_stage_mask = vk::PipelineStageFlags{detail::get_pipeline_stage_flags(new_layout)};
    const auto src_access_mask = vk::AccessFlags{detail::get_access_flags(old_layout)};
    const auto dst_access_mask = vk::AccessFlags{detail::get_access_flags(new_layout)};

    image_layout_transition(command_buffer,
                            image,
                            src_stage_mask,
                            dst_stage_mask,
                            src_access_mask,
                            dst_access_mask,
                            old_layout,
                            new_layout,
                            subresource_range);
}

inline void image_layout_transition(const vk::CommandBuffer &command_buffer,
                                    const vk::Image &image,
                                    const vk::ImageLayout old_layout,
                                    const vk::ImageLayout new_layout) noexcept {
    constexpr auto subresource_range = vk::ImageSubresourceRange{/* .aspectMask = */ vk::ImageAspectFlagBits::eColor,
                                                                 /* .baseMipLevel = */ 0,
                                                                 /* .levelCount = */ 1,
                                                                 /* .baseArrayLayer = */ 0,
                                                                 /* .layerCount = */ 1};

    image_layout_transition(command_buffer, image, old_layout, new_layout, subresource_range);
}

inline void image_layout_transition(
        const vk::CommandBuffer command_buffer,
        const std::vector<std::pair<vk::Image, vk::ImageSubresourceRange>> &images_and_ranges,
        const vk::ImageLayout old_layout,
        const vk::ImageLayout new_layout) {
    const auto src_stage_mask = vk::PipelineStageFlags{detail::get_pipeline_stage_flags(old_layout)};
    const auto dst_stage_mask = vk::PipelineStageFlags{detail::get_pipeline_stage_flags(new_layout)};
    const auto src_access_mask = vk::AccessFlags{detail::get_access_flags(old_layout)};
    const auto dst_access_mask = vk::AccessFlags{detail::get_access_flags(new_layout)};

    auto image_memory_barriers = std::vector<vk::ImageMemoryBarrier>{};
    image_memory_barriers.reserve(images_and_ranges.size());

    for (const auto &[image, subresource_range] : images_and_ranges) {
        image_memory_barriers.emplace_back(vk::ImageMemoryBarrier{/* .srcAccessMask = */ src_access_mask,
                                                                  /* .dstAccessMask = */ dst_access_mask,
                                                                  /* .oldLayout = */ old_layout,
                                                                  /* .newLayout = */ new_layout,
                                                                  /* .srcQueueFamilyIndex = */ vk::QueueFamilyIgnored,
                                                                  /* .dstQueueFamilyIndex = */ vk::QueueFamilyIgnored,
                                                                  /* .image = */ image,
                                                                  /* .subresourceRange = */ subresource_range});
    }

    command_buffer.pipelineBarrier(/* .srcStageMask = */ src_stage_mask,
                                   /* .dstStageMask = */ dst_stage_mask,
                                   /* .dependencyFlags = */ {}, // dependencyFlags
                                   /* .memoryBarriers = */ nullptr,
                                   /* .bufferMemoryBarriers = */ nullptr,
                                   /* .imageMemoryBarriers = */ image_memory_barriers);
}

} // namespace sm::arcane::vulkan