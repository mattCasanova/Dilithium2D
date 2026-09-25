#pragma once

#include <vulkan/vulkan.h>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

// Pure physical-device choice: Device.cpp (Phase 5) queries Vulkan into these facts, and these functions decide.
// No GPU calls, so they are unit tested without one.

namespace dilithium {

/// What one queue family offers, as far as device choice cares.
struct QueueFamilyFacts {
    VkQueueFlags flags = 0;
    bool canPresent = false; ///< `vkGetPhysicalDeviceSurfaceSupportKHR` for our surface
};

/// Everything device choice needs to know about one physical device.
struct DeviceFacts {
    std::string name;
    VkPhysicalDeviceType type = VK_PHYSICAL_DEVICE_TYPE_OTHER;
    uint32_t apiVersion = 0;
    bool hasSwapchainExtension = false;
    bool dynamicRendering = false; ///< from `VkPhysicalDeviceVulkan13Features`
    bool synchronization2 = false; ///< from `VkPhysicalDeviceVulkan13Features`
    std::vector<QueueFamilyFacts> queueFamilies;
    uint32_t surfaceFormatCount = 0;
    uint32_t presentModeCount = 0;
};

struct UsableDevice {
    int score = 0;
    uint32_t queueFamily = 0; ///< does both graphics and present
};

struct UnusableDevice {
    std::vector<std::string> reasons; ///< every failed requirement, never empty
};

using DeviceVerdict = std::variant<UsableDevice, UnusableDevice>;

/// Discrete 1000, integrated 100, anything else 10.
int deviceTypeScore(VkPhysicalDeviceType type);

/// "discrete GPU", "integrated GPU", ... for the device log.
std::string_view deviceTypeName(VkPhysicalDeviceType type);

/// A packed Vulkan version as text: "1.4.357".
std::string formatApiVersion(uint32_t version);

/// The first family that does both graphics and present. Every real GPU has one; we require it rather than juggle
/// two queues.
std::optional<uint32_t> findQueueFamily(std::span<const QueueFamilyFacts> families);

/// Usable, with its score and queue family, or unusable, with every reason why. Requires Vulkan 1.3,
/// `VK_KHR_swapchain`, `dynamicRendering`, `synchronization2`, a graphics + present family, and a surface that offers
/// at least one format and one present mode.
DeviceVerdict scoreDevice(const DeviceFacts& device);

/// The index of the highest-scoring usable device; the first one wins a tie. `std::nullopt` when none is usable.
std::optional<std::size_t> pickDevice(std::span<const DeviceVerdict> verdicts);

/// One line for the device log: "usable, score 100, queue family 0" or "unusable: no VK_KHR_swapchain; ...".
std::string describeVerdict(const DeviceVerdict& verdict);

} // namespace dilithium
