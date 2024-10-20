#include "application.hpp"

#include <fmt/core.h>
#include <spdlog/spdlog.h>

namespace sm::arcane {

void Application::run() {
    fmt::print("Project: {}\n"
               "Version: {}.{}.{}\n",
               m_app_config.title,
               m_app_config.version.major,
               m_app_config.version.minor,
               m_app_config.version.patch);

    spdlog::info("Showing Windows window");
    while (!m_window.should_close()) {
        m_window.pool_events();
    }
}

} // namespace sm::arcane