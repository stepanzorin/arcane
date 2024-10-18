// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include <cstdint>
#include <string_view>

namespace sm::arcane {

struct app_config_s {
private:
    struct version_levels_s {
        std::uint8_t major = SM_ARCANE_PROJECT_VERSION_MAJOR;
        std::uint8_t minor = SM_ARCANE_PROJECT_VERSION_MINOR;
        std::uint8_t patch = SM_ARCANE_PROJECT_VERSION_PATCH;
    };

public:
    std::string_view title = SM_ARCANE_PROJECT_NAME;
    version_levels_s version = {};
};

} // namespace sm::arcane