// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include <chrono>
#include <cstdint>

namespace sm::arcane {

struct frame_info_s {
    std::uint32_t frame_index = 0;
    std::uint32_t image_index = 0;
    std::chrono::steady_clock::time_point started_time{};
};

struct prev_frame_info_s {
    float finished_time = 0.0f;
};

} // namespace sm::arcane