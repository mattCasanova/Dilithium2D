#include "gfx/Allocator.hpp"

#include "gfx/VkCheck.hpp"

namespace dilithium {

Allocator::Allocator(VkInstance instance, VkPhysicalDevice physical, VkDevice device) {
    const VmaAllocatorCreateInfo info{
        .physicalDevice = physical,
        .device = device,
        .instance = instance,
        .vulkanApiVersion = VK_API_VERSION_1_3,
    };
    KHAAAN(vmaCreateAllocator(&info, &m_allocator));
}

Allocator::~Allocator() {
    vmaDestroyAllocator(m_allocator);
}

} // namespace dilithium
