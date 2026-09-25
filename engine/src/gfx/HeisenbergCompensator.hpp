#pragma once

#include <vulkan/vulkan.h>

#include <array>
#include <cstdint>

namespace dilithium {

inline constexpr uint32_t kFramesInFlight = 2;

/// What one frame in flight owns. The CPU records into one slot while the GPU may still be running the other.
struct FrameSlot {
    VkCommandPool pool = VK_NULL_HANDLE;         ///< reset whole each frame: simpler and cheaper than per buffer
    VkCommandBuffer commands = VK_NULL_HANDLE;   ///< one primary buffer, allocated from `pool`
    VkFence inFlight = VK_NULL_HANDLE;           ///< signaled when the GPU finishes this slot's submit; starts signaled
    VkSemaphore imageAvailable = VK_NULL_HANDLE; ///< signaled by vkAcquireNextImageKHR, waited on by the submit
};

/// The Heisenberg compensator: the per-frame sync objects that let the CPU record one frame while the GPU draws the
/// other (LiquidMetal2D's BufferProvider semaphore, per frame instead of per buffer). How does it work? Very well,
/// thank you. Not copyable or movable: WarpCore builds it in place.
class HeisenbergCompensator {
public:
    HeisenbergCompensator(VkDevice device, uint32_t queueFamily);
    ~HeisenbergCompensator();

    HeisenbergCompensator(const HeisenbergCompensator&) = delete;
    HeisenbergCompensator& operator=(const HeisenbergCompensator&) = delete;
    HeisenbergCompensator(HeisenbergCompensator&&) = delete;
    HeisenbergCompensator& operator=(HeisenbergCompensator&&) = delete;

    const FrameSlot& current() const { return m_slots[m_index]; }
    void advance() { m_index = (m_index + 1) % kFramesInFlight; }

private:
    void destroy();

    VkDevice m_device;
    std::array<FrameSlot, kFramesInFlight> m_slots{};
    uint32_t m_index = 0;
};

} // namespace dilithium
