#include "gfx/HeisenbergCompensator.hpp"

#include "gfx/VkCheck.hpp"

namespace dilithium {

HeisenbergCompensator::HeisenbergCompensator(VkDevice device, uint32_t queueFamily) : m_device(device) {
    // A throw part-way would skip the destructor, so clean up by hand on the way out.
    try {
        for (FrameSlot& slot : m_slots) {
            const VkCommandPoolCreateInfo poolInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, // its buffers live one frame
                .queueFamilyIndex = queueFamily,
            };
            KHAAAN(vkCreateCommandPool(device, &poolInfo, nullptr, &slot.pool));

            const VkCommandBufferAllocateInfo allocateInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool = slot.pool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1,
            };
            KHAAAN(vkAllocateCommandBuffers(device, &allocateInfo, &slot.commands));

            // Signaled, so the first wait on each slot returns at once.
            const VkFenceCreateInfo fenceInfo{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                                              .flags = VK_FENCE_CREATE_SIGNALED_BIT};
            KHAAAN(vkCreateFence(device, &fenceInfo, nullptr, &slot.inFlight));

            const VkSemaphoreCreateInfo semaphoreInfo{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
            KHAAAN(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &slot.imageAvailable));
        }
    } catch (...) {
        destroy();
        throw;
    }
}

HeisenbergCompensator::~HeisenbergCompensator() {
    destroy();
}

void HeisenbergCompensator::destroy() {
    // Destroying a pool frees its command buffers. Each call accepts VK_NULL_HANDLE, for a slot that was never made.
    for (FrameSlot& slot : m_slots) {
        vkDestroySemaphore(m_device, slot.imageAvailable, nullptr);
        vkDestroyFence(m_device, slot.inFlight, nullptr);
        vkDestroyCommandPool(m_device, slot.pool, nullptr);
        slot = FrameSlot{};
    }
}

} // namespace dilithium
