// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include <array>
#include <chrono>
#include <cstdint>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "vulkan/device.hpp"
#include "vulkan/swapchain.hpp"
#include "window.hpp"

namespace sm::arcane {

inline static constexpr auto g_max_frames_in_flight = 2u;

struct frame_context_s {
    vk::raii::Semaphore image_available_semaphore = nullptr;
    vk::raii::Semaphore render_finished_semaphore = nullptr;
    vk::raii::Fence in_flight_fence = nullptr;
};

class Renderer {
public:
    Renderer(vulkan::Device &device, const Window &window, const vulkan::Swapchain &swapchain)
        : m_device{device},
          m_window{window},
          m_swapchain{swapchain} {}

private:
    vulkan::Device &m_device;
    const Window &m_window;
    const vulkan::Swapchain &m_swapchain;

    // Frame management variables
    std::uint32_t m_current_frame_index = 0; // Host frame index
    std::uint32_t m_image_index = 0; // Swapchain image index
    float m_frame_time = 0.0f; // Time between frames

    std::array<frame_context_s, g_max_frames_in_flight> m_frames;
    std::vector<vk::raii::Fence> m_images_in_flight{}; // One fence per swapchain image

    // Timing variables for frame time calculation
    std::chrono::steady_clock::time_point m_last_frame_time{};
};

} // namespace sm::arcane