// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "window.hpp"

namespace sm::arcane::vulkan {

class Instance {
public:
    explicit Instance(const app_config_s &config);

    [[nodiscard]] const vk::raii::Instance &handle() const noexcept { return m_instance; }

    [[nodiscard]] vk::raii::SurfaceKHR create_surface(const Window &window) {
        return {m_instance, window.create_surface(*m_instance)};
    }

    [[nodiscard]] std::vector<vk::raii::PhysicalDevice> enumerate_physical_devices() const noexcept {
        return vk::raii::PhysicalDevices(m_instance);
    }

private:
    vk::raii::Context m_context;
    vk::raii::Instance m_instance;

#ifdef SM_ARCANE_DEBUG_MODE
    vk::raii::DebugUtilsMessengerEXT m_debug_messenger;
#endif
};

} // namespace sm::arcane::vulkan
