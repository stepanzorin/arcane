#include "application.hpp"

#include <fmt/core.h>

namespace sm::arcane {

void Application::run() {
    fmt::print("Project: {}\n"
               "Version: {}.{}.{}",
               m_app_config.title,
               m_app_config.version.major,
               m_app_config.version.minor,
               m_app_config.version.patch);
}

} // namespace sm::arcane