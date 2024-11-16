#include "application.hpp"

namespace sm::arcane {

Application::Application(const app_config_s &config)
    : m_logger{spdlog::default_logger()->clone("app")},
      m_window{config},
      m_instance{config},
      m_surface{m_instance.create_surface(m_window)},
      m_device{m_instance.handle(), m_surface},
      m_swapchain_uptr{create_swapchain()},
      m_renderer{m_device, *m_swapchain_uptr, m_logger->clone("renderer")},
      m_scene{std::make_optional<scene::Scene>(m_window, m_device, m_swapchain_uptr->aspect_ratio())} {}

void Application::run() {
    while (!m_window.should_close()) {
        m_window.pool_events();
        m_scene->update();
        m_renderer.render({*m_scene});
    }
    m_device.device().waitIdle();
}

std::unique_ptr<vulkan::Swapchain> Application::create_swapchain() {
    return std::make_unique<vulkan::Swapchain>(m_device, m_window, m_surface);
}

void Application::recreate_swapchain() {
    m_swapchain_uptr = std::make_unique<vulkan::Swapchain>(m_device, m_window, m_surface, m_swapchain_uptr.get());
}

} // namespace sm::arcane