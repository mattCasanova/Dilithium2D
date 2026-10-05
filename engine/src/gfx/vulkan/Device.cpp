#include "gfx/vulkan/Device.hpp"

#include "gfx/vulkan/DeviceChoices.hpp"
#include "gfx/vulkan/ExtensionChoices.hpp"
#include "gfx/vulkan/VkCheck.hpp"

#include <dilithium/utilities/Assert.hpp>
#include <dilithium/utilities/Log.hpp>

#include <cstddef>
#include <cstdint>
#include <format>
#include <optional>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

namespace dilithium {
namespace {

/// One GPU as the loader reports it: the facts device choice needs, plus what the log shows.
struct Candidate {
    VkPhysicalDevice physical = VK_NULL_HANDLE;
    DeviceFacts facts;
    std::string driver; ///< "MoltenVK 1.4.2"; empty for a device older than Vulkan 1.3
};

std::vector<QueueFamilyFacts> queueFamilyFacts(VkPhysicalDevice physical, VkSurfaceKHR surface) {
    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical, &count, nullptr);
    std::vector<VkQueueFamilyProperties> families(count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical, &count, families.data());

    std::vector<QueueFamilyFacts> facts;
    for (uint32_t index = 0; index < count; ++index) {
        VkBool32 canPresent = VK_FALSE;
        VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(physical, index, surface, &canPresent));
        facts.push_back({families[index].queueFlags, canPresent == VK_TRUE});
    }
    return facts;
}

Candidate describe(VkPhysicalDevice physical, VkSurfaceKHR surface) {
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physical, &properties);

    Candidate candidate{.physical = physical};
    DeviceFacts& facts = candidate.facts;
    facts.name = properties.deviceName;
    facts.type = properties.deviceType;
    facts.apiVersion = properties.apiVersion;
    if (facts.apiVersion < VK_API_VERSION_1_3) {
        // Unusable, and the spec forbids asking it for 1.2 or 1.3 structures: its verdict gives the reason.
        return candidate;
    }

    VkPhysicalDeviceDriverProperties driver{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES};
    VkPhysicalDeviceProperties2 properties2{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2, .pNext = &driver};
    vkGetPhysicalDeviceProperties2(physical, &properties2);
    candidate.driver = std::format("{} {}", driver.driverName, driver.driverInfo);

    VkPhysicalDeviceVulkan13Features features13{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
    VkPhysicalDeviceFeatures2 features{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .pNext = &features13};
    vkGetPhysicalDeviceFeatures2(physical, &features);
    facts.dynamicRendering = features13.dynamicRendering == VK_TRUE;
    facts.synchronization2 = features13.synchronization2 == VK_TRUE;

    const auto extensions = vkEnumerate<VkExtensionProperties>(
        "vkEnumerateDeviceExtensionProperties", [physical](uint32_t* count, VkExtensionProperties* items) {
            return vkEnumerateDeviceExtensionProperties(physical, nullptr, count, items);
        });
    facts.hasSwapchainExtension = hasExtension(extensions, VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    facts.queueFamilies = queueFamilyFacts(physical, surface);

    // Counts only: the swapchain (Phase 6) asks again for the lists themselves.
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physical, surface, &facts.surfaceFormatCount, nullptr));
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physical, surface, &facts.presentModeCount, nullptr));
    return candidate;
}

} // namespace

Device::Device(VkInstance instance, VkSurfaceKHR surface) {
    const auto physicalDevices = vkEnumerate<VkPhysicalDevice>(
        "vkEnumeratePhysicalDevices", [instance](uint32_t* count, VkPhysicalDevice* items) {
            return vkEnumeratePhysicalDevices(instance, count, items);
        });

    std::vector<Candidate> candidates;
    std::vector<DeviceVerdict> verdicts;
    for (VkPhysicalDevice candidate : physicalDevices) {
        candidates.push_back(describe(candidate, surface));
        verdicts.push_back(scoreDevice(candidates.back().facts));
    }
    for (std::size_t index = 0; index < candidates.size(); ++index) {
        const Candidate& candidate = candidates[index];
        logInfo("GPU {}: {} ({}, Vulkan {}{}{}): {}", index, candidate.facts.name, deviceTypeName(candidate.facts.type),
                formatApiVersion(candidate.facts.apiVersion), candidate.driver.empty() ? "" : ", ", candidate.driver,
                describeVerdict(verdicts[index]));
    }

    const std::optional<std::size_t> best = pickDevice(verdicts);
    if (!best) {
        throw std::runtime_error(physicalDevices.empty() ? "Vulkan found no GPU at all"
                                                         : "no GPU meets Dilithium2D's needs (the log says why)");
    }
    const Candidate& chosen = candidates[*best];
    const auto* usable = std::get_if<UsableDevice>(&verdicts[*best]);
    DILITHIUM_ASSERT(usable != nullptr, "pickDevice chose an unusable device");
    physical = chosen.physical;
    queueFamily = usable->queueFamily;

    const auto offered = vkEnumerate<VkExtensionProperties>(
        "vkEnumerateDeviceExtensionProperties", [this](uint32_t* count, VkExtensionProperties* items) {
            return vkEnumerateDeviceExtensionProperties(physical, nullptr, count, items);
        });
    const std::vector<const char*> extensions = planDeviceExtensions(offered);

    const float priority = 1.0f;
    const VkDeviceQueueCreateInfo queueInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = queueFamily,
        .queueCount = 1,
        .pQueuePriorities = &priority,
    };
    VkPhysicalDeviceVulkan13Features enable13{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .synchronization2 = VK_TRUE,
        .dynamicRendering = VK_TRUE,
    };
    const VkPhysicalDeviceFeatures2 enable{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .pNext = &enable13};
    const VkDeviceCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &enable,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueInfo,
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
    };
    VK_CHECK(vkCreateDevice(physical, &info, nullptr, &device));
    vkGetDeviceQueue(device, queueFamily, 0, &queue);
    logInfo("using GPU {}: {}, queue family {}", *best, chosen.facts.name, queueFamily);
}

Device::~Device() {
    vkDestroyDevice(device, nullptr);
}

} // namespace dilithium
