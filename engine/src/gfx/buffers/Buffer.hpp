#pragma once

#include <dilithium/utilities/NonCopyable.hpp>

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <cstddef>
#include <span>

namespace dilithium {

/// A `VkBuffer` and the memory VMA gave it, destroyed together. Move-only: moving hands the handles over and leaves
/// the source empty. D2 needs host-visible buffers only (the CPU writes, the GPU reads, every frame); device-local
/// buffers filled through a staging copy arrive with D3's textures.
class Buffer : NonCopyable {
public:
    /// A buffer the CPU can write straight into: persistently mapped, with memory VMA picks for sequential host
    /// writes. `usage` says what the GPU will do with it (`VK_BUFFER_USAGE_VERTEX_BUFFER_BIT`, ...).
    static Buffer hostVisible(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage);

    Buffer() = default; ///< empty: no handles, nothing to destroy
    ~Buffer();

    Buffer(Buffer&& other) noexcept;
    Buffer& operator=(Buffer&& other) noexcept;

    /// Copies `bytes` to the start of the mapped memory. More than the buffer holds is a programmer error.
    void write(std::span<const std::byte> bytes);

    [[nodiscard]] VkBuffer handle() const { return buffer; }
    [[nodiscard]] VkDeviceSize getSize() const { return size; }

private:
    Buffer(VmaAllocator newAllocator, VkBuffer newBuffer, VmaAllocation newAllocation, VkDeviceSize newSize,
           void* newMapped);
    void destroy();

    VmaAllocator allocator = VK_NULL_HANDLE;
    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    VkDeviceSize size = 0;
    void* mapped = nullptr; ///< where the CPU writes; valid for the buffer's whole life
};

} // namespace dilithium
