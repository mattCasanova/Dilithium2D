#include "gfx/vulkan/ExtensionChoices.hpp"

#include <algorithm>

namespace dilithium {
namespace {

void addOnce(std::vector<const char*>& names, const char* name) {
    const bool present =
        std::ranges::any_of(names, [name](const char* existing) { return std::string_view(existing) == name; });
    if (!present) {
        names.push_back(name);
    }
}

} // namespace

bool hasExtension(std::span<const VkExtensionProperties> offered, std::string_view name) {
    return std::ranges::any_of(offered, [name](const VkExtensionProperties& extension) {
        return std::string_view(extension.extensionName) == name;
    });
}

bool hasLayer(std::span<const VkLayerProperties> offered, std::string_view name) {
    return std::ranges::any_of(
        offered, [name](const VkLayerProperties& layer) { return std::string_view(layer.layerName) == name; });
}

InstanceExtensionPlan planInstanceExtensions(std::span<const char* const> sdlRequired,
                                             std::span<const VkExtensionProperties> offered,
                                             std::span<const VkExtensionProperties> layerOffered, bool validation) {
    InstanceExtensionPlan plan;
    for (const char* name : sdlRequired) {
        if (!hasExtension(offered, name)) {
            plan.missing.emplace_back(name);
        }
        addOnce(plan.enable, name);
    }
    if (hasExtension(offered, kPortabilityEnumeration)) {
        addOnce(plan.enable, kPortabilityEnumeration);
    }
    if (validation) {
        const auto available = [&](const char* name) {
            return hasExtension(offered, name) || hasExtension(layerOffered, name);
        };
        if (!available(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
            plan.missing.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        addOnce(plan.enable, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        if (available(kLayerSettings)) {
            addOnce(plan.enable, kLayerSettings);
            plan.layerSettings = true;
        }
    }
    plan.portabilityEnumeration = std::ranges::any_of(
        plan.enable, [](const char* name) { return std::string_view(name) == kPortabilityEnumeration; });
    return plan;
}

std::vector<const char*> planDeviceExtensions(std::span<const VkExtensionProperties> offered) {
    std::vector<const char*> enable{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    if (hasExtension(offered, kPortabilitySubset)) {
        enable.push_back(kPortabilitySubset);
    }
    return enable;
}

} // namespace dilithium
