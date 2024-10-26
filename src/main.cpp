#include <cstdlib>
#include <exception>

#include <boost/system/system_error.hpp>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>

#include "app_config.hpp"
#include "application.hpp"
#include "util/filesystem_helpers.hpp"

namespace sm::arcane {

void run(const std::string_view config_name, const std::shared_ptr<spdlog::logger> &global_logger) noexcept try {
    try {
        const auto config_path = util::application_directory_path() / config_name;
        global_logger->trace("Configuration path: {}", config_path.string());
        const auto config = app_config_from_json(config_path);
        global_logger->trace("{} is running", config.title);

        Application app{config};
        app.run();
    } catch (const boost::system::system_error &ex) {
        global_logger->critical("The application crashed due to failure config parsing. Reason: {}", ex.what());
    }

    global_logger->info("arcane successful finished");
} catch (const std::exception &ex) {
    global_logger->critical("The application crashed due to an unhandled exception. Reason: {}", ex.what());
} catch (...) {
    global_logger->critical("The application crashed due to an unhandled exception. Reason: <unknown exception>");
}

} // namespace sm::arcane

int main(const int argc, char *argv[]) noexcept {
    const auto global_logger = spdlog::default_logger()->clone("global");
    global_logger->set_level(spdlog::level::trace);

    if (argc == 1) {
        global_logger->critical("It is necessary to pass only one input argument to the program, which is the name "
                                "configuration file in JSON format. The config must be in root app directory.");
        return EXIT_FAILURE;
    }
    const char *config_name = argv[1];

    sm::arcane::run(config_name, global_logger);
    return EXIT_SUCCESS;
}