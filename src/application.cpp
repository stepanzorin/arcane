#include "application.hpp"

#include <spdlog/spdlog.h>

namespace sm::arcane {

Application::Application(const app_config_s &config)
    : m_logger{spdlog::default_logger()->clone("app")},
      m_window{config},
      m_instance{config},
      m_surface{m_instance.create_surface(m_window)},
      m_device{m_instance.handle(), m_surface} {}

void Application::run() {
    while (!m_window.should_close()) {
        m_window.pool_events();
    }
}

} // namespace sm::arcane