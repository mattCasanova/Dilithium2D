#pragma once

#include <vk_mem_alloc.h>

namespace dilithium {

/// The VMA allocator for every buffer and image. Nothing is allocated in D1; creating it now fixes its place in the
/// teardown order (after the device is made, destroyed before it). Not copyable or movable: WarpCore builds it in
/// place.
class Allocator {
public:
    Allocator(VkInstance instance, VkPhysicalDevice physical, VkDevice device);
    ~Allocator();

    Allocator(const Allocator&) = delete;
    Allocator& operator=(const Allocator&) = delete;
    Allocator(Allocator&&) = delete;
    Allocator& operator=(Allocator&&) = delete;

    VmaAllocator handle() const { return m_allocator; }

private:
    VmaAllocator m_allocator = VK_NULL_HANDLE;
};

} // namespace dilithium
