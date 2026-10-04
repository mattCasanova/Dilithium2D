#include "gfx/buffers/Buffer.hpp"

#include "gfx/vulkan/VkCheck.hpp"

#include <dilithium/core/Assert.hpp>

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <cstddef>
#include <cstring>
#include <span>
#include <utility>

namespace dilithium {

Buffer Buffer::hostVisible(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage) {
    const VkBufferCreateInfo bufferInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    // SEQUENTIAL_WRITE: VMA picks host-visible memory and, where it exists, the kind the CPU writes fastest
    // (write-combined). MAPPED: mapped once, for the buffer's whole life.
    const VmaAllocationCreateInfo allocationInfo{
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO,
    };
    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    VmaAllocationInfo result{};
    VK_CHECK(vmaCreateBuffer(allocator, &bufferInfo, &allocationInfo, &buffer, &allocation, &result));
    return Buffer{allocator, buffer, allocation, size, result.pMappedData};
}

Buffer::Buffer(VmaAllocator allocator, VkBuffer buffer, VmaAllocation allocation, VkDeviceSize size, void* mapped)
    : m_allocator(allocator), m_buffer(buffer), m_allocation(allocation), m_size(size), m_mapped(mapped) {}

Buffer::~Buffer() {
    destroy();
}

Buffer::Buffer(Buffer&& other) noexcept
    : m_allocator(std::exchange(other.m_allocator, VK_NULL_HANDLE)),
      m_buffer(std::exchange(other.m_buffer, VK_NULL_HANDLE)),
      m_allocation(std::exchange(other.m_allocation, VK_NULL_HANDLE)), m_size(std::exchange(other.m_size, 0)),
      m_mapped(std::exchange(other.m_mapped, nullptr)) {}

Buffer& Buffer::operator=(Buffer&& other) noexcept {
    if (this != &other) {
        destroy();
        m_allocator = std::exchange(other.m_allocator, VK_NULL_HANDLE);
        m_buffer = std::exchange(other.m_buffer, VK_NULL_HANDLE);
        m_allocation = std::exchange(other.m_allocation, VK_NULL_HANDLE);
        m_size = std::exchange(other.m_size, 0);
        m_mapped = std::exchange(other.m_mapped, nullptr);
    }
    return *this;
}

void Buffer::write(std::span<const std::byte> bytes) {
    if (bytes.size() > m_size) {
        DILITHIUM_UNREACHABLE("writing more bytes than the buffer holds");
    }
    if (m_mapped == nullptr) {
        DILITHIUM_UNREACHABLE("writing to a buffer that is not mapped");
    }
    std::memcpy(m_mapped, bytes.data(), bytes.size());
    // Host-visible memory need not be coherent; a flush makes the write visible to the GPU. VMA makes it a no-op
    // where the memory is coherent, as it is on MoltenVK.
    VK_CHECK(vmaFlushAllocation(m_allocator, m_allocation, 0, bytes.size()));
}

void Buffer::destroy() {
    if (m_buffer != VK_NULL_HANDLE) {
        // Unmaps too: the MAPPED flag's mapping belongs to the allocation.
        vmaDestroyBuffer(m_allocator, m_buffer, m_allocation);
        m_buffer = VK_NULL_HANDLE;
        m_allocation = VK_NULL_HANDLE;
        m_mapped = nullptr;
    }
}

} // namespace dilithium
