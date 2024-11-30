// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include <vulkan/vulkan.hpp>

namespace sm::arcane::render::passes {

inline constexpr auto g_color_subresource_range = vk::ImageSubresourceRange{
        vk::ImageAspectFlagBits::eColor,
        0,
        vk::RemainingMipLevels,
        0,
        vk::RemainingArrayLayers,
};

inline constexpr auto g_depth_subresource_range = vk::ImageSubresourceRange{
        vk::ImageAspectFlagBits::eDepth,
        0,
        vk::RemainingMipLevels,
        0,
        vk::RemainingArrayLayers,
};

} // namespace sm::arcane::render::passes