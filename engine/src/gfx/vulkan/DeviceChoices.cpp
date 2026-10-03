#include "gfx/vulkan/DeviceChoices.hpp"

#include <dilithium/core/Log.hpp>

#include <cstddef>
#include <cstdint>
#include <format>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace dilithium {
namespace {

// A discrete GPU beats an integrated one beats everything else, by a margin no tie-break can cross.
constexpr int kDiscreteGpuScore = 1000;
constexpr int kIntegratedGpuScore = 100;
constexpr int kOtherDeviceScore = 10;

} // namespace

int deviceTypeScore(VkPhysicalDeviceType type) {
    switch (type) {
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        return kDiscreteGpuScore;
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        return kIntegratedGpuScore;
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
    case VK_PHYSICAL_DEVICE_TYPE_CPU:
    case VK_PHYSICAL_DEVICE_TYPE_OTHER:
        return kOtherDeviceScore;
    case VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM:
        break;
    }
    // A type newer than these headers is the driver's news, not our bug: say so, then treat it as "other".
    logWarning("unknown VkPhysicalDeviceType {}; scoring it as 'other'", static_cast<int>(type));
    return kOtherDeviceScore;
}

std::string_view deviceTypeName(VkPhysicalDeviceType type) {
    switch (type) {
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        return "discrete GPU";
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        return "integrated GPU";
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
        return "virtual GPU";
    case VK_PHYSICAL_DEVICE_TYPE_CPU:
        return "CPU";
    case VK_PHYSICAL_DEVICE_TYPE_OTHER:
        return "other";
    case VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM:
        break;
    }
    // deviceTypeScore already warns about a type newer than our headers; the log just needs a word.
    return "unknown device type";
}

std::string formatApiVersion(uint32_t version) {
    return std::format("{}.{}.{}", VK_API_VERSION_MAJOR(version), VK_API_VERSION_MINOR(version),
                       VK_API_VERSION_PATCH(version));
}

std::optional<uint32_t> findQueueFamily(std::span<const QueueFamilyFacts> families) {
    for (std::size_t index = 0; index < families.size(); ++index) {
        const QueueFamilyFacts& family = families[index];
        if ((family.flags & VK_QUEUE_GRAPHICS_BIT) != 0 && family.canPresent) {
            return static_cast<uint32_t>(index);
        }
    }
    return std::nullopt;
}

DeviceVerdict scoreDevice(const DeviceFacts& device) {
    std::vector<std::string> reasons;
    if (device.apiVersion < VK_API_VERSION_1_3) {
        reasons.push_back(std::format("Vulkan {}.{} (needs 1.3)", VK_API_VERSION_MAJOR(device.apiVersion),
                                      VK_API_VERSION_MINOR(device.apiVersion)));
    }
    if (!device.hasSwapchainExtension) {
        reasons.emplace_back("no VK_KHR_swapchain");
    }
    if (!device.dynamicRendering) {
        reasons.emplace_back("no dynamicRendering");
    }
    if (!device.synchronization2) {
        reasons.emplace_back("no synchronization2");
    }
    const std::optional<uint32_t> queueFamily = findQueueFamily(device.queueFamilies);
    if (!queueFamily) {
        reasons.emplace_back("no queue family does both graphics and present");
    }
    if (device.surfaceFormatCount == 0) {
        reasons.emplace_back("the surface offers no formats");
    }
    if (device.presentModeCount == 0) {
        reasons.emplace_back("the surface offers no present modes");
    }

    // A missing queue family is already in `reasons`; naming it again here lets a reader (and the linter) see that
    // the optional below is never empty.
    if (!reasons.empty() || !queueFamily) {
        return UnusableDevice{std::move(reasons)};
    }
    return UsableDevice{deviceTypeScore(device.type), *queueFamily};
}

std::optional<std::size_t> pickDevice(std::span<const DeviceVerdict> verdicts) {
    std::optional<std::size_t> best;
    int bestScore = 0;
    for (std::size_t index = 0; index < verdicts.size(); ++index) {
        const auto* usable = std::get_if<UsableDevice>(&verdicts[index]);
        if (usable != nullptr && (!best || usable->score > bestScore)) {
            best = index;
            bestScore = usable->score;
        }
    }
    return best;
}

std::string describeVerdict(const DeviceVerdict& verdict) {
    if (const auto* usable = std::get_if<UsableDevice>(&verdict)) {
        return std::format("usable, score {}, queue family {}", usable->score, usable->queueFamily);
    }
    std::string line = "unusable:";
    const char* separator = " ";
    for (const std::string& reason : std::get<UnusableDevice>(verdict).reasons) {
        line += separator;
        line += reason;
        separator = "; ";
    }
    return line;
}

} // namespace dilithium
