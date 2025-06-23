// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024-2025 Stepan Zorin <stz.hom@gmail.com>

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

template<typename Handle>
struct resource_state_handle_s {
    Handle handle;
};

using image_resource_state_t = resource_state_handle_s<vulkan::DeviceMemoryImage>;

struct gpu_resources_s {
    struct gbuffer_s {
        image_resource_state_t albedo;
        image_resource_state_t normal;
        image_resource_state_t specular;
        image_resource_state_t albedo_roughness;
        image_resource_state_t ambient_occlusion;
        image_resource_state_t metalness;
        image_resource_state_t roughness;
    } gbuffer;

    image_resource_state_t depth_stencil;

    image_resource_state_t hdr;
};

} // namespace sm::arcane::render::passes