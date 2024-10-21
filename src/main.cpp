#include <cstdlib>
#include <exception>

#include <boost/system/system_error.hpp>
#include <spdlog/spdlog.h>

#include "app_config.hpp"
#include "application.hpp"

namespace sm::arcane {

void run(const std::shared_ptr<spdlog::logger> &global_logger) noexcept try {
    global_logger->info("arcane is running");

    try {
        const auto config = app_config_from_json();
        Application app{config};
        app.run();
    } catch (const boost::system::system_error& ex) {
        global_logger->critical("The application crashed due to failure config parsing. Reason: {}", ex.what());
    }

    global_logger->info("arcane successful finished");
}  catch (const std::exception &ex) {
    global_logger->critical("The application crashed due to an unhandled exception. Reason: {}", ex.what());
} catch (...) {
    global_logger->critical("The application crashed due to an unhandled exception. Reason: <unknown exception>");
}

} // namespace sm::arcane

int main() noexcept {
    const auto global_logger = spdlog::default_logger()->clone("global");
    sm::arcane::run(global_logger);
    return EXIT_SUCCESS;
}