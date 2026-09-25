#include "gfx/ExtensionChoices.hpp"

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstring>
#include <string>
#include <string_view>
#include <vector>

using dilithium::hasExtension;
using dilithium::hasLayer;
using dilithium::InstanceExtensionPlan;
using dilithium::planDeviceExtensions;
using dilithium::planInstanceExtensions;

namespace {

VkExtensionProperties extension(const char* name) {
    VkExtensionProperties properties{};
    std::strncpy(properties.extensionName, name, VK_MAX_EXTENSION_NAME_SIZE - 1);
    return properties;
}

VkLayerProperties layer(const char* name) {
    VkLayerProperties properties{};
    std::strncpy(properties.layerName, name, VK_MAX_EXTENSION_NAME_SIZE - 1);
    return properties;
}

std::vector<std::string> names(const std::vector<const char*>& list) {
    return {list.begin(), list.end()};
}

/// What brew's loader offers on the M4 Max, for the parts that matter here.
const std::vector<VkExtensionProperties> kMacOffered{
    extension("VK_KHR_surface"),
    extension("VK_EXT_metal_surface"),
    extension("VK_KHR_portability_enumeration"),
    extension("VK_EXT_debug_utils"),
};

const std::array<const char*, 2> kSdlOnMac{"VK_KHR_surface", "VK_EXT_metal_surface"};

} // namespace

TEST_CASE("hasExtension and hasLayer match whole names", "[extensions]") {
    CHECK(hasExtension(kMacOffered, "VK_EXT_debug_utils"));
    CHECK_FALSE(hasExtension(kMacOffered, "VK_EXT_debug"));
    const std::vector<VkLayerProperties> layers{layer("VK_LAYER_KHRONOS_validation")};
    CHECK(hasLayer(layers, "VK_LAYER_KHRONOS_validation"));
    CHECK_FALSE(hasLayer(layers, "VK_LAYER_KHRONOS"));
}

TEST_CASE("on the Mac: SDL's extensions, portability enumeration, and debug utils with validation", "[extensions]") {
    const InstanceExtensionPlan plan = planInstanceExtensions(kSdlOnMac, kMacOffered, true);
    CHECK(names(plan.enable) == std::vector<std::string>{"VK_KHR_surface", "VK_EXT_metal_surface",
                                                         "VK_KHR_portability_enumeration", "VK_EXT_debug_utils"});
    CHECK(plan.portabilityEnumeration);
    CHECK(plan.missing.empty());
}

TEST_CASE("without validation, no debug utils", "[extensions]") {
    const InstanceExtensionPlan plan = planInstanceExtensions(kSdlOnMac, kMacOffered, false);
    CHECK(names(plan.enable) ==
          std::vector<std::string>{"VK_KHR_surface", "VK_EXT_metal_surface", "VK_KHR_portability_enumeration"});
}

TEST_CASE("where the loader offers no portability enumeration (a native driver), it is left out", "[extensions]") {
    const std::vector<VkExtensionProperties> nativeDriver{extension("VK_KHR_surface"), extension("VK_KHR_xcb_surface"),
                                                          extension("VK_EXT_debug_utils")};
    const std::array<const char*, 2> sdl{"VK_KHR_surface", "VK_KHR_xcb_surface"};
    const InstanceExtensionPlan plan = planInstanceExtensions(sdl, nativeDriver, true);
    CHECK(names(plan.enable) == std::vector<std::string>{"VK_KHR_surface", "VK_KHR_xcb_surface", "VK_EXT_debug_utils"});
    CHECK_FALSE(plan.portabilityEnumeration);
    CHECK(plan.missing.empty());
}

TEST_CASE("an extension SDL lists itself is not added twice", "[extensions]") {
    const std::array<const char*, 3> sdl{"VK_KHR_surface", "VK_EXT_metal_surface", "VK_KHR_portability_enumeration"};
    const InstanceExtensionPlan plan = planInstanceExtensions(sdl, kMacOffered, false);
    CHECK(names(plan.enable) ==
          std::vector<std::string>{"VK_KHR_surface", "VK_EXT_metal_surface", "VK_KHR_portability_enumeration"});
    CHECK(plan.portabilityEnumeration);
}

TEST_CASE("a required extension the loader does not offer is reported missing", "[extensions]") {
    const std::vector<VkExtensionProperties> bare{extension("VK_KHR_surface")};
    const InstanceExtensionPlan plan = planInstanceExtensions(kSdlOnMac, bare, true);
    CHECK(plan.missing == std::vector<std::string>{"VK_EXT_metal_surface", "VK_EXT_debug_utils"});
}

TEST_CASE("device extensions: swapchain, plus portability subset when offered", "[extensions]") {
    const std::vector<VkExtensionProperties> moltenVk{extension("VK_KHR_swapchain"),
                                                      extension("VK_KHR_portability_subset")};
    CHECK(names(planDeviceExtensions(moltenVk)) ==
          std::vector<std::string>{"VK_KHR_swapchain", "VK_KHR_portability_subset"});
    const std::vector<VkExtensionProperties> native{extension("VK_KHR_swapchain")};
    CHECK(names(planDeviceExtensions(native)) == std::vector<std::string>{"VK_KHR_swapchain"});
}
