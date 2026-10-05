#include "gfx/vulkan/Allocator.hpp"

#include "gfx/vulkan/VkCheck.hpp"

namespace dilithium {

Allocator::Allocator(VkInstance instance, VkPhysicalDevice physical, VkDevice device) {
    const VmaAllocatorCreateInfo info{
        .physicalDevice = physical,
        .device = device,
        .instance = instance,
        .vulkanApiVersion = VK_API_VERSION_1_3,
    };
    VK_CHECK(vmaCreateAllocator(&info, &allocator));
}

Allocator::~Allocator() {
    vmaDestroyAllocator(allocator);
}

} // namespace dilithium
