#pragma once

#include <dilithium/utilities/NonMovable.hpp>

#include <vk_mem_alloc.h>

namespace dilithium {

/// The VMA allocator for every buffer and image. Nothing is allocated in D1; creating it now fixes its place in the
/// teardown order (after the device is made, destroyed before it). Not copyable or movable: RenderCore builds it in
/// place.
class Allocator : NonMovable {
public:
    Allocator(VkInstance instance, VkPhysicalDevice physical, VkDevice device);
    ~Allocator();

    [[nodiscard]] VmaAllocator handle() const { return allocator; }

private:
    VmaAllocator allocator = VK_NULL_HANDLE;
};

} // namespace dilithium
