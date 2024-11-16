// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include "vulkan/swapchain.hpp"

namespace sm::arcane::render {

// структура для передачи в более глубокие уровни вызовов
struct render_resources_s {
    vulkan::Swapchain swapchain;
    // depth image & view
    // hdr image & view
    // other intermediate images and views, buffers, etc.
};

} // namespace sm::arcane::render