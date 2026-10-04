#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <cstddef>
#include <span>

namespace dilithium {

/// A `VkBuffer` and the memory VMA gave it, destroyed together. Move-only: moving hands the handles over and leaves
/// the source empty. D2 needs host-visible buffers only (the CPU writes, the GPU reads, every frame); device-local
/// buffers filled through a staging copy arrive with D3's textures.
class Buffer {
public:
    /// A buffer the CPU can write straight into: persistently mapped, with memory VMA picks for sequential host
    /// writes. `usage` says what the GPU will do with it (`VK_BUFFER_USAGE_VERTEX_BUFFER_BIT`, ...).
    static Buffer hostVisible(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage);

    Buffer() = default; ///< empty: no handles, nothing to destroy
    ~Buffer();

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;
    Buffer(Buffer&& other) noexcept;
    Buffer& operator=(Buffer&& other) noexcept;

    /// Copies `bytes` to the start of the mapped memory. More than the buffer holds is a programmer error.
    void write(std::span<const std::byte> bytes);

    [[nodiscard]] VkBuffer handle() const { return m_buffer; }
    [[nodiscard]] VkDeviceSize size() const { return m_size; }

private:
    Buffer(VmaAllocator allocator, VkBuffer buffer, VmaAllocation allocation, VkDeviceSize size, void* mapped);
    void destroy();

    VmaAllocator m_allocator = VK_NULL_HANDLE;
    VkBuffer m_buffer = VK_NULL_HANDLE;
    VmaAllocation m_allocation = VK_NULL_HANDLE;
    VkDeviceSize m_size = 0;
    void* m_mapped = nullptr; ///< where the CPU writes; valid for the buffer's whole life
};

} // namespace dilithium
