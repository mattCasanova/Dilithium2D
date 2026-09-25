#include "gfx/DeviceChoices.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <variant>
#include <vector>

using dilithium::describeVerdict;
using dilithium::DeviceFacts;
using dilithium::deviceTypeScore;
using dilithium::DeviceVerdict;
using dilithium::findQueueFamily;
using dilithium::pickDevice;
using dilithium::QueueFamilyFacts;
using dilithium::scoreDevice;
using dilithium::UnusableDevice;
using dilithium::UsableDevice;

namespace {

using Reasons = std::vector<std::string>;

constexpr VkQueueFlags kAllRounder = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;

/// The M4 Max as MoltenVK 1.4.2 reports it (vulkaninfo, 2026-09-25): usable.
DeviceFacts appleM4Max() {
    DeviceFacts device;
    device.name = "Apple M4 Max";
    device.type = VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
    device.apiVersion = VK_MAKE_API_VERSION(0, 1, 4, 357);
    device.hasSwapchainExtension = true;
    device.dynamicRendering = true;
    device.synchronization2 = true;
    device.queueFamilies = {{kAllRounder, true}, {kAllRounder, true}, {kAllRounder, true}, {kAllRounder, true}};
    device.surfaceFormatCount = 60;
    device.presentModeCount = 2;
    return device;
}

Reasons reasonsFor(const DeviceFacts& device) {
    const DeviceVerdict verdict = scoreDevice(device);
    const auto* unusable = std::get_if<UnusableDevice>(&verdict);
    REQUIRE(unusable != nullptr);
    return unusable->reasons;
}

} // namespace

TEST_CASE("a device with everything is usable, with its score and queue family", "[device]") {
    const DeviceVerdict verdict = scoreDevice(appleM4Max());
    const auto* usable = std::get_if<UsableDevice>(&verdict);
    REQUIRE(usable != nullptr);
    CHECK(usable->score == 100);
    CHECK(usable->queueFamily == 0);
}

TEST_CASE("discrete outscores integrated outscores everything else", "[device]") {
    CHECK(deviceTypeScore(VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) == 1000);
    CHECK(deviceTypeScore(VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) == 100);
    CHECK(deviceTypeScore(VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU) == 10);
    CHECK(deviceTypeScore(VK_PHYSICAL_DEVICE_TYPE_CPU) == 10);
    CHECK(deviceTypeScore(VK_PHYSICAL_DEVICE_TYPE_OTHER) == 10);
}

TEST_CASE("a device type newer than our headers scores as other", "[device]") {
    CHECK(deviceTypeScore(static_cast<VkPhysicalDeviceType>(42)) == 10);
}

TEST_CASE("a Vulkan 1.2 device is unusable, and the reason says so", "[device]") {
    DeviceFacts device = appleM4Max();
    device.apiVersion = VK_MAKE_API_VERSION(0, 1, 2, 283);
    CHECK(reasonsFor(device) == Reasons{"Vulkan 1.2 (needs 1.3)"});
}

TEST_CASE("each missing requirement makes a device unusable, with its own reason", "[device]") {
    DeviceFacts device = appleM4Max();

    SECTION("swapchain extension") {
        device.hasSwapchainExtension = false;
        CHECK(reasonsFor(device) == Reasons{"no VK_KHR_swapchain"});
    }
    SECTION("dynamic rendering") {
        device.dynamicRendering = false;
        CHECK(reasonsFor(device) == Reasons{"no dynamicRendering"});
    }
    SECTION("synchronization2") {
        device.synchronization2 = false;
        CHECK(reasonsFor(device) == Reasons{"no synchronization2"});
    }
    SECTION("one family for graphics and present") {
        // Graphics and present exist, but never in the same family.
        device.queueFamilies = {{VK_QUEUE_GRAPHICS_BIT, false}, {VK_QUEUE_TRANSFER_BIT, true}};
        CHECK(reasonsFor(device) == Reasons{"no queue family does both graphics and present"});
    }
    SECTION("surface formats") {
        device.surfaceFormatCount = 0;
        CHECK(reasonsFor(device) == Reasons{"the surface offers no formats"});
    }
    SECTION("present modes") {
        device.presentModeCount = 0;
        CHECK(reasonsFor(device) == Reasons{"the surface offers no present modes"});
    }
}

TEST_CASE("an unusable device lists every reason, not just the first", "[device]") {
    CHECK(reasonsFor(DeviceFacts{}) == Reasons{
                                           "Vulkan 0.0 (needs 1.3)",
                                           "no VK_KHR_swapchain",
                                           "no dynamicRendering",
                                           "no synchronization2",
                                           "no queue family does both graphics and present",
                                           "the surface offers no formats",
                                           "the surface offers no present modes",
                                       });
}

TEST_CASE("findQueueFamily picks the first family with both graphics and present", "[device]") {
    const std::vector<QueueFamilyFacts> families{
        {VK_QUEUE_GRAPHICS_BIT, false},
        {VK_QUEUE_TRANSFER_BIT, true},
        {VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, true},
        {VK_QUEUE_GRAPHICS_BIT, true},
    };
    CHECK(findQueueFamily(families) == 2u);
}

TEST_CASE("findQueueFamily without a graphics + present family is nullopt", "[device]") {
    const std::vector<QueueFamilyFacts> families{{VK_QUEUE_GRAPHICS_BIT, false}, {VK_QUEUE_COMPUTE_BIT, true}};
    CHECK_FALSE(findQueueFamily(families).has_value());
    CHECK_FALSE(findQueueFamily({}).has_value());
}

TEST_CASE("pickDevice takes the highest-scoring usable device; the first wins a tie", "[device]") {
    const std::vector<DeviceVerdict> verdicts{
        UnusableDevice{{"no VK_KHR_swapchain"}},
        UsableDevice{100, 0},
        UsableDevice{1000, 1},
        UsableDevice{1000, 0},
    };
    CHECK(pickDevice(verdicts) == 2u);
}

TEST_CASE("an unusable discrete GPU never beats a usable integrated one", "[device]") {
    DeviceFacts discrete = appleM4Max();
    discrete.type = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
    discrete.hasSwapchainExtension = false;
    const std::vector<DeviceVerdict> verdicts{scoreDevice(discrete), scoreDevice(appleM4Max())};
    CHECK(pickDevice(verdicts) == 1u);
}

TEST_CASE("pickDevice with nothing usable is nullopt", "[device]") {
    const std::vector<DeviceVerdict> verdicts{UnusableDevice{{"no VK_KHR_swapchain"}}};
    CHECK_FALSE(pickDevice(verdicts).has_value());
    CHECK_FALSE(pickDevice({}).has_value());
}

TEST_CASE("describeVerdict gives one log line either way", "[device]") {
    CHECK(describeVerdict(UsableDevice{100, 0}) == "usable, score 100, queue family 0");
    CHECK(describeVerdict(UnusableDevice{{"no VK_KHR_swapchain", "no dynamicRendering"}}) ==
          "unusable: no VK_KHR_swapchain; no dynamicRendering");
}
