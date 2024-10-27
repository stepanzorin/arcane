// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>

namespace sm::arcane::common {

/* Note: Samples are very preliminary. No calibration required  */
/* NB: But the first 4 are guaranteed to be correct */

struct linear_repeat_sampler_s {
    vk::Filter mag_filter = vk::Filter::eLinear;
    vk::Filter min_filter = vk::Filter::eLinear;
    vk::SamplerMipmapMode mipmap_mode = vk::SamplerMipmapMode::eLinear;
    vk::SamplerAddressMode address_mode_u = vk::SamplerAddressMode::eRepeat;
    vk::SamplerAddressMode address_mode_v = vk::SamplerAddressMode::eRepeat;
    vk::SamplerAddressMode address_mode_w = vk::SamplerAddressMode::eRepeat;
};

struct linear_clamp_sampler_s {
    vk::Filter mag_filter = vk::Filter::eLinear;
    vk::Filter min_filter = vk::Filter::eLinear;
    vk::SamplerMipmapMode mipmap_mode = vk::SamplerMipmapMode::eLinear;
    vk::SamplerAddressMode address_mode_u = vk::SamplerAddressMode::eClampToEdge;
    vk::SamplerAddressMode address_mode_v = vk::SamplerAddressMode::eClampToEdge;
    vk::SamplerAddressMode address_mode_w = vk::SamplerAddressMode::eClampToEdge;
};

struct nearest_repeat_sampler_s {
    vk::Filter mag_filter = vk::Filter::eNearest;
    vk::Filter min_filter = vk::Filter::eNearest;
    vk::SamplerMipmapMode mipmap_mode = vk::SamplerMipmapMode::eNearest;
    vk::SamplerAddressMode address_mode_u = vk::SamplerAddressMode::eRepeat;
    vk::SamplerAddressMode address_mode_v = vk::SamplerAddressMode::eRepeat;
    vk::SamplerAddressMode address_mode_w = vk::SamplerAddressMode::eRepeat;
};

struct nearest_clamp_sampler_s {
    vk::Filter mag_filter = vk::Filter::eNearest;
    vk::Filter min_filter = vk::Filter::eNearest;
    vk::SamplerMipmapMode mipmap_mode = vk::SamplerMipmapMode::eNearest;
    vk::SamplerAddressMode address_mode_u = vk::SamplerAddressMode::eClampToEdge;
    vk::SamplerAddressMode address_mode_v = vk::SamplerAddressMode::eClampToEdge;
    vk::SamplerAddressMode address_mode_w = vk::SamplerAddressMode::eClampToEdge;
};

// Shadow sampler using depth test
struct shadow_map_sampler_s {
    vk::Filter mag_filter = vk::Filter::eLinear;
    vk::Filter min_filter = vk::Filter::eLinear;
    vk::SamplerMipmapMode mipmap_mode = vk::SamplerMipmapMode::eNearest;
    vk::SamplerAddressMode address_mode_u = vk::SamplerAddressMode::eClampToEdge;
    vk::SamplerAddressMode address_mode_v = vk::SamplerAddressMode::eClampToEdge;
    vk::SamplerAddressMode address_mode_w = vk::SamplerAddressMode::eClampToEdge;
    bool enable_compare = true;
    vk::CompareOp compare_op = vk::CompareOp::eLess; // Used to render shadows
};

template<float MaxAnisotropy = 4.0f, bool EnabledAnisotropy = true>
struct bilinear_repeat_sampler_s : linear_repeat_sampler_s {
    bool enable_anisotropy = EnabledAnisotropy;
};

template<float MaxAnisotropy = 16.0f, bool Enabled = true>
struct anisotropic_repeat_sampler_s : linear_repeat_sampler_s {
    float max_anisotropy = MaxAnisotropy;
    bool enabled = Enabled;
};

template<float MaxAnisotropy = 8.0f, bool Enabled = true>
using normal_map_sampler_s = anisotropic_repeat_sampler_s<MaxAnisotropy, Enabled>;

template<float MaxAnisotropy = 4.0f, bool Enabled = true>
struct cube_map_sampler_s : linear_clamp_sampler_s {
    float max_anisotropy = MaxAnisotropy;
    bool enable_anisotropy = Enabled;
};

} // namespace sm::arcane::common