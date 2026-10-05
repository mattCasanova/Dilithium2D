#include "gfx/vulkan/Instance.hpp"

#include "gfx/vulkan/DeviceChoices.hpp"
#include "gfx/vulkan/ExtensionChoices.hpp"
#include "gfx/vulkan/VkCheck.hpp"
#include "platform/WindowSurface.hpp"

#include <dilithium/utilities/Log.hpp>

#include <array>
#include <cstdint>
#include <cstdlib>
#include <format>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace dilithium {
namespace {

bool validationWanted() {
#ifdef DILITHIUM_DEBUG
    const char* optOut = std::getenv("DILITHIUM_NO_VALIDATION");
    if (optOut != nullptr && std::string_view(optOut) == "1") {
        logWarning(
            "DILITHIUM_NO_VALIDATION=1: running WITHOUT the validation layer. Zero messages proves nothing now.");
        return false;
    }
    return true;
#else
    return false;
#endif
}

std::string_view messageKind(VkDebugUtilsMessageTypeFlagsEXT types) {
    if ((types & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) != 0) {
        return "validation";
    }
    if ((types & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) != 0) {
        return "validation (performance)";
    }
    return "validation (general)";
}

VKAPI_ATTR VkBool32 VKAPI_CALL onValidationMessage(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                   VkDebugUtilsMessageTypeFlagsEXT types,
                                                   const VkDebugUtilsMessengerCallbackDataEXT* data, void* userData) {
    auto& log = *static_cast<ValidationLog*>(userData);
    const char* message = data != nullptr && data->pMessage != nullptr ? data->pMessage : "(no message text)";
    const bool isError = (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) != 0;
    if (isError) {
        ++log.errors;
        logError("{}: {}", messageKind(types), message);
    } else {
        ++log.warnings;
        logWarning("{}: {}", messageKind(types), message);
    }
#ifdef DILITHIUM_DEBUG
    if (isError) {
        std::abort(); // stop inside the Vulkan call that caused it, so the debugger's stack points there
    }
#endif
    return VK_FALSE; // never ask Vulkan to fail the call; we want to see the real behavior
}

VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo(ValidationLog* log) {
    return {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = onValidationMessage,
        .pUserData = log,
    };
}

std::string joined(std::span<const char* const> names) {
    std::string line;
    for (const char* name : names) {
        line += line.empty() ? "" : ", ";
        line += name;
    }
    return line;
}

std::string joined(std::span<const std::string> names) {
    std::string line;
    for (const std::string& name : names) {
        line += line.empty() ? "" : ", ";
        line += name;
    }
    return line;
}

/// The loader's Vulkan version, which must be 1.3 or newer.
uint32_t checkLoaderVersion() {
    uint32_t loaderVersion = 0;
    VK_CHECK(vkEnumerateInstanceVersion(&loaderVersion));
    if (loaderVersion < VK_API_VERSION_1_3) {
        throw std::runtime_error(
            std::format("the Vulkan loader is {}; Dilithium2D needs 1.3", formatApiVersion(loaderVersion)));
    }
    return loaderVersion;
}

/// The extensions the validation layer itself adds. Throws when the layer is not installed.
std::vector<VkExtensionProperties> validationLayerExtensions() {
    const auto layers = vkEnumerate<VkLayerProperties>(
        "vkEnumerateInstanceLayerProperties",
        [](uint32_t* count, VkLayerProperties* items) { return vkEnumerateInstanceLayerProperties(count, items); });
    if (!hasLayer(layers, kValidationLayer)) {
        throw std::runtime_error("the Vulkan validation layer is not installed (brew install "
                                 "vulkan-validationlayers), or set DILITHIUM_NO_VALIDATION=1 to run without it");
    }
    return vkEnumerate<VkExtensionProperties>(
        "vkEnumerateInstanceExtensionProperties", [](uint32_t* count, VkExtensionProperties* items) {
            return vkEnumerateInstanceExtensionProperties(kValidationLayer, count, items);
        });
}

struct Messenger {
    VkDebugUtilsMessengerEXT handle = VK_NULL_HANDLE;
    PFN_vkDestroyDebugUtilsMessengerEXT destroy = nullptr;
};

/// Creates the debug messenger through the extension's function pointers. Throws if the extension is enabled but
/// its functions are missing.
Messenger createMessenger(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT& info) {
    // vkGetInstanceProcAddr returns one generic function pointer type; reinterpret_cast is how Vulkan hands out
    // extension functions.
    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
    auto create = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
    Messenger messenger{.destroy = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                            vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"))};
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
    if (create == nullptr || messenger.destroy == nullptr) {
        throw std::runtime_error("VK_EXT_debug_utils is enabled but its functions are missing");
    }
    VK_CHECK(create(instance, &info, nullptr, &messenger.handle));
    return messenger;
}

} // namespace

Instance::Instance() {
    const uint32_t loaderVersion = checkLoaderVersion();
    const bool validation = validationWanted();
    std::vector<VkExtensionProperties> layerOffered; // what the validation layer itself adds
    if (validation) {
        layerOffered = validationLayerExtensions();
        validationLog = std::make_unique<ValidationLog>();
    }

    const std::span<const char* const> windowExtensions = windowInstanceExtensions();
    const auto offered = vkEnumerate<VkExtensionProperties>(
        "vkEnumerateInstanceExtensionProperties", [](uint32_t* count, VkExtensionProperties* items) {
            return vkEnumerateInstanceExtensionProperties(nullptr, count, items);
        });
    const InstanceExtensionPlan plan = planInstanceExtensions(windowExtensions, offered, layerOffered, validation);
    if (!plan.missing.empty()) {
        throw std::runtime_error(std::format("Vulkan instance extensions missing: {}", joined(plan.missing)));
    }
    if (validation && !plan.layerSettings) {
        logWarning(
            "the validation layer takes no settings ({} missing): synchronization and best-practices checks are off",
            kLayerSettings);
    }
    const char* validationState = "off";
    if (validation) {
        validationState = plan.layerSettings ? "on, with synchronization and best-practices checks" : "on";
    }
    logInfo("Vulkan loader {}, validation {}", formatApiVersion(loaderVersion), validationState);
    DILITHIUM_LOG_DEBUG("instance extensions: {}", joined(plan.enable));

    const VkApplicationInfo application{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Dilithium2D",
        .pEngineName = "Dilithium2D",
        .apiVersion = VK_API_VERSION_1_3,
    };
    // A copy of the messenger's create-info rides on the instance's own, so instance creation and destruction are
    // checked too, before and after the real messenger exists.
    const VkDebugUtilsMessengerCreateInfoEXT messengerInfo = messengerCreateInfo(validationLog.get());
    // Two checks the layer has off by default. Synchronization validation checks that barriers and semaphores really
    // order the GPU's work, which core validation does not. Best-practices validation warns about legal but poor use
    // (a wrong memory type, a redundant barrier); its warnings count like any other, so a run must stay silent.
    const VkBool32 enabled = VK_TRUE;
    const std::array<VkLayerSettingEXT, 2> settings{{
        {
            .pLayerName = kValidationLayer,
            .pSettingName = "validate_sync",
            .type = VK_LAYER_SETTING_TYPE_BOOL32_EXT,
            .valueCount = 1,
            .pValues = &enabled,
        },
        {
            .pLayerName = kValidationLayer,
            .pSettingName = "validate_best_practices",
            .type = VK_LAYER_SETTING_TYPE_BOOL32_EXT,
            .valueCount = 1,
            .pValues = &enabled,
        },
    }};
    const VkLayerSettingsCreateInfoEXT layerSettings{
        .sType = VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT,
        .pNext = &messengerInfo,
        .settingCount = static_cast<uint32_t>(settings.size()),
        .pSettings = settings.data(),
    };
    const void* chain = nullptr;
    if (validation) {
        chain = plan.layerSettings ? static_cast<const void*>(&layerSettings) : &messengerInfo;
    }
    const char* const layer = kValidationLayer;
    const VkInstanceCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = chain,
        .flags = plan.portabilityEnumeration ? VkInstanceCreateFlags{VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR}
                                             : VkInstanceCreateFlags{0},
        .pApplicationInfo = &application,
        .enabledLayerCount = validation ? 1u : 0u,
        .ppEnabledLayerNames = validation ? &layer : nullptr,
        .enabledExtensionCount = static_cast<uint32_t>(plan.enable.size()),
        .ppEnabledExtensionNames = plan.enable.data(),
    };
    VK_CHECK(vkCreateInstance(&info, nullptr, &instance));

    if (!validation) {
        return;
    }
    // The instance exists now, and a throw from here on would skip the destructor: destroy it by hand on the way out.
    try {
        const Messenger created = createMessenger(instance, messengerInfo);
        messenger = created.handle;
        destroyMessenger = created.destroy;
    } catch (...) {
        vkDestroyInstance(instance, nullptr);
        throw;
    }
}

// NOLINTNEXTLINE(bugprone-exception-escape): it logs; std::format's bad_alloc at shutdown may end the program, rightly
Instance::~Instance() {
    if (messenger != VK_NULL_HANDLE) {
        destroyMessenger(instance, messenger, nullptr);
    }
    vkDestroyInstance(instance, nullptr);
    if (validationLog) {
        // Reported after vkDestroyInstance, so it counts shutdown too.
        const ValidationLog& log = *validationLog;
        if (log.errors + log.warnings == 0) {
            logInfo("validation reported nothing");
        } else {
            logWarning("validation reported {} errors and {} warnings", log.errors, log.warnings);
        }
    }
}

} // namespace dilithium
