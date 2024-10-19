#include <cstdlib>
#include <exception>

#include <spdlog/spdlog.h>

#include "app_config.hpp"
#include "application.hpp"

using namespace sm::arcane;

int main() try {
    Application app{app_config_from_json()};
    app.run();

    return EXIT_SUCCESS;
} catch (const std::exception &ex) {
    spdlog::critical("The application crashed due to an unhandled exception. Reason: {}", ex.what());
    return EXIT_FAILURE;
} catch (...) {
    spdlog::critical("The application crashed due to an unhandled exception. Reason: <unknown exception>");
    return EXIT_FAILURE;
}