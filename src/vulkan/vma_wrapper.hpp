// Arcane (https://github.com/stepanzorin/arcane)
// Copyright Text: 2024 Stepan Zorin <stz.hom@gmail.com>

#pragma once

#include <concepts>
#include <cstddef>

#include <vk_mem_alloc.h>
#include <vulkan/vulkan_raii.hpp>

// !!! This is a `Beta Implementation`. It means the implementation might be several times improved
// TODO: improve if necessary

namespace sm::arcane::vulkan::vma {

class Allocation {
public:
    Allocation(VmaAllocator allocator, VmaAllocation handle) noexcept;

    Allocation(Allocation &&other) noexcept
        : m_allocator{std::exchange(other.m_allocator, nullptr)},
          m_handle{std::exchange(other.m_handle, nullptr)} {}

    [[nodiscard]] VmaAllocationInfo info() const;

    void flush(std::size_t offset = 0, std::size_t size = 0) const;

    void invalidate(std::size_t offset = 0, std::size_t size = 0) const;

private:
    template<typename Handle, typename Deleter>
    friend class AllocatedHandle;

    VmaAllocator m_allocator;
    VmaAllocation m_handle;
};

template<typename Handle, typename Deleter>
class AllocatedHandle {
public:
    AllocatedHandle(Allocation allocation, Handle handle, const Deleter deleter = Deleter{})
        : m_allocation{std::move(allocation)},
          m_handle{std::move(handle)},
          m_deleter{deleter} {}

    AllocatedHandle(AllocatedHandle &&other) noexcept
        : m_allocation{std::move(other.m_allocation)},
          m_handle{std::exchange(other.m_handle, Handle{})},
          m_deleter{std::move(other.m_deleter)} {}

    AllocatedHandle &operator=(AllocatedHandle &&other) noexcept {
        if (this != &other) {
            reset();
            m_allocation = std::move(other.m_allocation);
            m_handle = std::exchange(other.m_handle, Handle{});
            m_deleter = std::move(other.m_deleter);
        }
        return *this;
    }

    [[nodiscard]] VmaAllocationInfo allocation_info() const { return m_allocation.info(); }

    void flush(const std::size_t offset = 0, const std::size_t size = 0) const { m_allocation.flush(offset, size); }

    void invalidate(const std::size_t offset = 0, const std::size_t size = 0) const {
        m_allocation.invalidate(offset, size);
    }

    ~AllocatedHandle() { reset(); }

private:
    void reset() {
        if (m_handle) {
            m_deleter.destroy(m_allocation.m_allocator, m_handle, m_allocation.m_handle);
            m_handle = Handle{};
        }
    }

    Allocation m_allocation;
    Handle m_handle;
    Deleter m_deleter;
};

namespace detail {

struct buffer_deleter {
    static void destroy(const VmaAllocator allocator, const vk::Buffer buffer, const VmaAllocation allocation) {
        vmaDestroyBuffer(allocator, buffer, allocation);
    }
};

struct image_deleter {
    static void destroy(const VmaAllocator allocator, const vk::Image image, const VmaAllocation allocation) {
        vmaDestroyImage(allocator, image, allocation);
    }
};

} // namespace detail

using allocated_buffer_t = AllocatedHandle<vk::Buffer, detail::buffer_deleter>;
using allocated_image_t = AllocatedHandle<vk::Image, detail::image_deleter>;

class Allocator {
public:
    Allocator(vk::Instance instance, vk::PhysicalDevice physical_device, vk::Device device);

    [[nodiscard]] allocated_buffer_t allocate_buffer(const vk::BufferCreateInfo &buffer_create_info) {
        vk::Buffer buffer;
        auto allocation = allocate_memory_and_create_handle(buffer_create_info, buffer);
        return {std::move(allocation), buffer};
    }

    [[nodiscard]] allocated_image_t allocate_image(const vk::ImageCreateInfo &image_create_info) {
        vk::Image image;
        auto allocation = allocate_memory_and_create_handle(image_create_info, image);
        return {std::move(allocation), image};
    }

    ~Allocator() { vmaDestroyAllocator(m_allocator); }

private:
    template<typename Handle, typename HandleCreateInfo>
    Allocation allocate_memory_and_create_handle(const HandleCreateInfo &create_info, Handle &handle) {
        auto alloc_info = VmaAllocationCreateInfo{};
        alloc_info.usage = VMA_MEMORY_USAGE_AUTO;

        VmaAllocation allocation;
        vk::Result allocate_result;

        if constexpr (std::same_as<Handle, vk::Buffer>) {
            allocate_result = static_cast<vk::Result>(
                    vmaCreateBuffer(m_allocator,
                                    reinterpret_cast<const VkBufferCreateInfo *>(&create_info),
                                    &alloc_info,
                                    reinterpret_cast<VkBuffer *>(&handle),
                                    &allocation,
                                    nullptr));
        } else if constexpr (std::same_as<Handle, vk::Image>) {
            allocate_result = static_cast<vk::Result>(
                    vmaCreateImage(m_allocator,
                                   reinterpret_cast<const VkImageCreateInfo *>(&create_info),
                                   &alloc_info,
                                   reinterpret_cast<VkImage *>(&handle),
                                   &allocation,
                                   nullptr));
        } else {
            throw std::runtime_error{"Unsupported handle type"};
        }

        if (allocate_result != vk::Result::eSuccess) {
            throw std::runtime_error{"Failed to allocate memory"};
        }

        return {m_allocator, allocation};
    }

    VmaAllocator m_allocator;
};

} // namespace sm::arcane::vulkan::vma