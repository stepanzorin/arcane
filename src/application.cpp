#include "application.hpp"

#include <spdlog/spdlog.h>

namespace sm::arcane {

Application::Application(const app_config_s &config)
    : m_logger{spdlog::default_logger()->clone("app")}, //
      m_window{config} {}

void Application::run() {
    while (!m_window.should_close()) {
        m_window.pool_events();
    }
}

Application::~Application() = default;

} // namespace sm::arcane