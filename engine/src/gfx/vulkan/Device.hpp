#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>
#include <string>

namespace dilithium {

/// The chosen GPU and the logical device on it, with one queue that does both graphics and present. Not copyable or
/// movable: RenderCore builds it in place.
class Device {
public:
    /// Logs every GPU with its verdict, picks the best usable one for `surface`, and creates the logical device.
    /// Throws `std::runtime_error` when no GPU is usable.
    Device(VkInstance instance, VkSurfaceKHR surface);
    ~Device();

    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;
    Device(Device&&) = delete;
    Device& operator=(Device&&) = delete;

    [[nodiscard]] VkPhysicalDevice physical() const { return m_physical; }
    [[nodiscard]] VkDevice handle() const { return m_device; }
    [[nodiscard]] VkQueue queue() const { return m_queue; }
    [[nodiscard]] uint32_t queueFamily() const { return m_queueFamily; }

private:
    VkPhysicalDevice m_physical = VK_NULL_HANDLE;
    uint32_t m_queueFamily = 0;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_queue = VK_NULL_HANDLE;
};

} // namespace dilithium
