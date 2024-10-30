#include "vma_wrapper.hpp"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

namespace sm::arcane::vulkan::vma {

namespace {

const auto vma_vulkan_functions = VmaVulkanFunctions{

#define SM_ARCANE_VMA_FUNCTION_NAME(function_name) .function_name = function_name

        SM_ARCANE_VMA_FUNCTION_NAME(vkGetPhysicalDeviceProperties),
        SM_ARCANE_VMA_FUNCTION_NAME(vkGetPhysicalDeviceMemoryProperties),
        SM_ARCANE_VMA_FUNCTION_NAME(vkAllocateMemory),
        SM_ARCANE_VMA_FUNCTION_NAME(vkFreeMemory),
        SM_ARCANE_VMA_FUNCTION_NAME(vkMapMemory),
        SM_ARCANE_VMA_FUNCTION_NAME(vkUnmapMemory),
        SM_ARCANE_VMA_FUNCTION_NAME(vkFlushMappedMemoryRanges),
        SM_ARCANE_VMA_FUNCTION_NAME(vkInvalidateMappedMemoryRanges),
        SM_ARCANE_VMA_FUNCTION_NAME(vkBindBufferMemory),
        SM_ARCANE_VMA_FUNCTION_NAME(vkBindImageMemory),
        SM_ARCANE_VMA_FUNCTION_NAME(vkGetBufferMemoryRequirements),
        SM_ARCANE_VMA_FUNCTION_NAME(vkGetImageMemoryRequirements),
        SM_ARCANE_VMA_FUNCTION_NAME(vkCreateBuffer),
        SM_ARCANE_VMA_FUNCTION_NAME(vkDestroyBuffer),
        SM_ARCANE_VMA_FUNCTION_NAME(vkCreateImage),
        SM_ARCANE_VMA_FUNCTION_NAME(vkDestroyImage),
        SM_ARCANE_VMA_FUNCTION_NAME(vkCmdCopyBuffer)

#undef SM_ARCANE_VMA_FUNCTION_NAME

};

} // namespace

Allocation::Allocation(const VmaAllocator allocator, const VmaAllocation handle) noexcept
    : m_allocator{allocator},
      m_handle{handle} {}

VmaAllocationInfo Allocation::info() const {
    auto info = VmaAllocationInfo{};
    vmaGetAllocationInfo(m_allocator, m_handle, &info);
    return info;
}

void Allocation::flush(const std::size_t offset, const std::size_t size) const {
    if (!vmaFlushAllocation(m_allocator, m_handle, offset, size)) {
        throw std::runtime_error{"Failed to flush allocation"};
    }
}

void Allocation::invalidate(const std::size_t offset, const std::size_t size) const {
    if (!vmaInvalidateAllocation(m_allocator, m_handle, offset, size)) {
        throw std::runtime_error{"Failed to invalidate allocation"};
    }
}

Allocator::Allocator(const vk::Instance instance, const vk::PhysicalDevice physical_device, const vk::Device device)
    : m_allocator{[&] {
          auto allocator = VmaAllocator{};
          const auto create_info = VmaAllocatorCreateInfo{.physicalDevice = physical_device,
                                                          .device = device,
                                                          /* .preferredLargeHeapBlockSize = , */
                                                          /* .pAllocationCallbacks = , */
                                                          /* .pDeviceMemoryCallbacks = , */
                                                          /* .pHeapSizeLimit = , */
                                                          .pVulkanFunctions = &vma_vulkan_functions,
                                                          .instance = instance,
                                                          .vulkanApiVersion = VK_API_VERSION_1_3};

          if (vmaCreateAllocator(&create_info, &allocator) != VK_SUCCESS) {
              throw std::runtime_error{"Failed to create vma::Allocator"};
          }
          return allocator;
      }()} {}

} // namespace sm::arcane::vulkan::vma