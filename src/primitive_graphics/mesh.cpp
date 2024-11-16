#include "mesh.hpp"

namespace sm::arcane::primitive_graphics {

namespace {

template<typename T>
[[nodiscard]] vulkan::DeviceMemoryBuffer fill_buffer_impl(const vulkan::Device &device,
                                                          const vk::CommandPool &command_pool,
                                                          const std::vector<T> &data,
                                                          const vk::BufferUsageFlags usages) {
    const vk::DeviceSize buffer_size = sizeof(T) * data.size();
    auto buffer = device.create_device_memory_buffer(usages | vk::BufferUsageFlagBits::eTransferDst,
                                                     buffer_size,
                                                     0,
                                                     vk::MemoryPropertyFlagBits::eDeviceLocal);

    buffer.upload(device.physical_device(),
                  device.device(),
                  device.queue_families().graphics.queue,
                  command_pool,
                  data,
                  sizeof(T));

    return buffer;
}

[[nodiscard]] vulkan::DeviceMemoryBuffer fill_vertex_buffer(const vulkan::Device &device,
                                                            const vk::CommandPool &command_pool,
                                                            const std::vector<Mesh::vertex_s> &vertices) {
    assert(vertices.size() >= 3 && "Mesh::vertex_s count must be at least 3");
    return fill_buffer_impl(device, command_pool, vertices, vk::BufferUsageFlagBits::eVertexBuffer);
}

[[nodiscard]] vulkan::DeviceMemoryBuffer fill_index_buffer(const vulkan::Device &device,
                                                           const vk::CommandPool &command_pool,
                                                           const std::vector<std::uint32_t> &indices) {
    if (indices.empty()) {
        return nullptr;
    }
    return fill_buffer_impl(device, command_pool, indices, vk::BufferUsageFlagBits::eIndexBuffer);
}

} // namespace

Mesh::Mesh(const vulkan::Device &device,
           const vk::CommandPool &command_pool,
           std::vector<vertex_s> &&vertices,
           std::vector<std::uint32_t> &&indices)
    : m_device{*device.device()},
      m_vertices{std::move(vertices)},
      m_vertex_count{static_cast<std::uint32_t>(m_vertices.size())},
      m_vertex_buffer{fill_vertex_buffer(device, command_pool, m_vertices)},
      m_indices{std::move(indices)},
      m_exists_index_buffer{!m_indices.empty()},
      m_index_count{static_cast<std::uint32_t>(m_indices.size())},
      m_index_buffer{m_indices.empty() ? nullptr : fill_index_buffer(device, command_pool, m_indices)} {}


void Mesh::bind(const vk::CommandBuffer command_buffer) const {
    const auto vertex_buffers = std::vector{*m_vertex_buffer.buffer};
    const auto offsets = std::vector<vk::DeviceSize>{0};

    command_buffer.bindVertexBuffers(0, 1, vertex_buffers.data(), offsets.data());

    if (m_exists_index_buffer) {
        command_buffer.bindIndexBuffer(*m_index_buffer.buffer, 0, vk::IndexType::eUint32);
    }
}

void Mesh::draw(const vk::CommandBuffer command_buffer) const {
    if (m_exists_index_buffer) {
        command_buffer.drawIndexed(m_index_count, 1, 0, 0, 0);
    } else {
        command_buffer.draw(m_vertex_count, 1, 0, 0);
    }
}

} // namespace sm::arcane::primitive_graphics