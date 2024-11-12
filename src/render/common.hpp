// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include <chrono>
#include <cstdint>

#include <vulkan/vulkan_raii.hpp>

#include "vulkan/swapchain.hpp"
#include "window.hpp"

namespace sm::arcane::render {

struct frame_info_s {
    std::uint32_t frame_index = 0;
    std::uint32_t image_index = 0;
    std::chrono::steady_clock::time_point started_time{};
};

struct prev_frame_info_s {
    float finished_time = 0.0f;
};

struct frame_context_s {
    vk::raii::Semaphore image_available_semaphore = nullptr;
    vk::raii::Semaphore render_finished_semaphore = nullptr;
    vk::raii::Fence in_flight_fence = nullptr;
};

// структура для передачи в более глубокие уровни вызовов
struct render_resources_s {
    vulkan::Swapchain swapchain;
    // depth image & view
    // hdr image & view
    // other intermediate images and views, buffers, etc.
};


} // namespace sm::arcane::render