// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include <filesystem>

namespace sm::arcane::util {

[[nodiscard]] inline std::filesystem::path application_directory_path() noexcept {
    return SM_ARCANE_APPLICATION_DIR_PATH;
}

} // namespace sm::arcane::util