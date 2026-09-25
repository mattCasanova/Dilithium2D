#pragma once

#include <vulkan/vulkan.h>

#include <span>
#include <string>
#include <string_view>
#include <vector>

// Pure choices of layers and extensions, over the lists Vulkan reports. No GPU calls, so they are unit tested
// without one.

namespace dilithium {

inline constexpr const char* kValidationLayer = "VK_LAYER_KHRONOS_validation";
inline constexpr const char* kPortabilityEnumeration = "VK_KHR_portability_enumeration";
inline constexpr const char* kPortabilitySubset = "VK_KHR_portability_subset"; // in vulkan_beta.h, so named here

bool hasExtension(std::span<const VkExtensionProperties> offered, std::string_view name);
bool hasLayer(std::span<const VkLayerProperties> offered, std::string_view name);

/// The instance extensions to turn on.
struct InstanceExtensionPlan {
    std::vector<const char*> enable;
    bool portabilityEnumeration = false; ///< also set `VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR`
    std::vector<std::string> missing;    ///< required but not offered: instance creation must fail
};

/// SDL's surface extensions (required); `VK_KHR_portability_enumeration` whenever the loader offers it (MoltenVK
/// shows no GPU without it); `VK_EXT_debug_utils` when validation is on (required then). No name appears twice.
InstanceExtensionPlan planInstanceExtensions(std::span<const char* const> sdlRequired,
                                             std::span<const VkExtensionProperties> offered, bool validation);

/// `VK_KHR_swapchain`, plus `VK_KHR_portability_subset` when the device offers it: the spec requires enabling it
/// then. The caller has already checked that the device has the swapchain extension.
std::vector<const char*> planDeviceExtensions(std::span<const VkExtensionProperties> offered);

} // namespace dilithium
