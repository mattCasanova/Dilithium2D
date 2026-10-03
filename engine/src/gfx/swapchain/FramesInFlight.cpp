#include "gfx/swapchain/FramesInFlight.hpp"

#include "gfx/vulkan/VkCheck.hpp"

namespace dilithium {

FramesInFlight::FramesInFlight(VkDevice device, uint32_t queueFamily) : m_device(device) {
    // A throw part-way would skip the destructor, so clean up by hand on the way out.
    try {
        for (FrameSlot& slot : m_slots) {
            const VkCommandPoolCreateInfo poolInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, // its buffers live one frame
                .queueFamilyIndex = queueFamily,
            };
            VK_CHECK(vkCreateCommandPool(device, &poolInfo, nullptr, &slot.pool));

            const VkCommandBufferAllocateInfo allocateInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool = slot.pool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1,
            };
            VK_CHECK(vkAllocateCommandBuffers(device, &allocateInfo, &slot.commands));

            // Signaled, so the first wait on each slot returns at once.
            const VkFenceCreateInfo fenceInfo{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                                              .flags = VK_FENCE_CREATE_SIGNALED_BIT};
            VK_CHECK(vkCreateFence(device, &fenceInfo, nullptr, &slot.inFlight));

            const VkSemaphoreCreateInfo semaphoreInfo{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
            VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &slot.imageAvailable));
        }
    } catch (...) {
        destroy();
        throw;
    }
}

FramesInFlight::~FramesInFlight() {
    destroy();
}

void FramesInFlight::destroy() {
    // Destroying a pool frees its command buffers. Each call accepts VK_NULL_HANDLE, for a slot that was never made.
    for (FrameSlot& slot : m_slots) {
        vkDestroySemaphore(m_device, slot.imageAvailable, nullptr);
        vkDestroyFence(m_device, slot.inFlight, nullptr);
        vkDestroyCommandPool(m_device, slot.pool, nullptr);
        slot = FrameSlot{};
    }
}

} // namespace dilithium
