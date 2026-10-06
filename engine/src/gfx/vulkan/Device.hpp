#pragma once

#include <dilithium/utilities/NonMovable.hpp>

#include <vulkan/vulkan.h>

#include <cstdint>
#include <string>

namespace dilithium {

/// The chosen GPU and the logical device on it, with one queue that does both graphics and present. Not copyable or
/// movable: RenderCore builds it in place.
class Device : NonMovable {
public:
    /// Logs every GPU with its verdict, picks the best usable one for `surface`, and creates the logical device.
    /// Throws `std::runtime_error` when no GPU is usable.
    Device(VkInstance instance, VkSurfaceKHR surface);
    ~Device();

    [[nodiscard]] VkPhysicalDevice getPhysical() const { return physical; }
    [[nodiscard]] VkDevice handle() const { return device; }
    [[nodiscard]] VkQueue getQueue() const { return queue; }
    [[nodiscard]] uint32_t getQueueFamily() const { return queueFamily; }

private:
    VkPhysicalDevice physical = VK_NULL_HANDLE;
    uint32_t queueFamily = 0;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue queue = VK_NULL_HANDLE;
};

} // namespace dilithium
