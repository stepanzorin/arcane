#include <stdexcept>

#include <fmt/printf.h>
#include <spdlog/spdlog.h>

#include "app_config.hpp"

using namespace sm::arcane;

int main() try {
    const auto app_config = app_config_from_json();
    fmt::print("Project: {}\n"
               "Version: {}.{}.{}",
               app_config.title,
               app_config.version.major,
               app_config.version.minor,
               app_config.version.patch);

    return EXIT_SUCCESS;
} catch (const std::exception &ex) {
    spdlog::critical("The application crashed due to an unhandled exception. Reason: {}", ex.what());
    return EXIT_FAILURE;
} catch (...) {
    spdlog::critical("The application crashed due to an unhandled exception. Reason: <unknown exception>");
    return EXIT_FAILURE;
}