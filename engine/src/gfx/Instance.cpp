#include "gfx/Instance.hpp"

#include "gfx/DeviceChoices.hpp"
#include "gfx/ExtensionChoices.hpp"
#include "gfx/VkCheck.hpp"

#include <dilithium/core/Log.hpp>

#include <SDL3/SDL_vulkan.h>

#include <cstdlib>
#include <format>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>

namespace dilithium {
namespace {

bool validationWanted() {
#if defined(DILITHIUM_DEBUG)
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
#if defined(DILITHIUM_DEBUG)
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

} // namespace

Instance::Instance() {
    uint32_t loaderVersion = 0;
    VK_CHECK(vkEnumerateInstanceVersion(&loaderVersion));
    if (loaderVersion < VK_API_VERSION_1_3) {
        throw std::runtime_error(
            std::format("the Vulkan loader is {}; Dilithium2D needs 1.3", formatApiVersion(loaderVersion)));
    }

    const bool validation = validationWanted();
    if (validation) {
        const auto layers = vkEnumerate<VkLayerProperties>(
            "vkEnumerateInstanceLayerProperties",
            [](uint32_t* count, VkLayerProperties* items) { return vkEnumerateInstanceLayerProperties(count, items); });
        if (!hasLayer(layers, kValidationLayer)) {
            throw std::runtime_error("the Vulkan validation layer is not installed (brew install "
                                     "vulkan-validationlayers), or set DILITHIUM_NO_VALIDATION=1 to run without it");
        }
        m_validationLog = std::make_unique<ValidationLog>();
    }
    logInfo("Vulkan loader {}, validation {}", formatApiVersion(loaderVersion), validation ? "on" : "off");

    Uint32 sdlCount = 0;
    const char* const* sdlExtensions = SDL_Vulkan_GetInstanceExtensions(&sdlCount);
    if (sdlExtensions == nullptr) {
        throw std::runtime_error(std::format("SDL_Vulkan_GetInstanceExtensions failed: {}", SDL_GetError()));
    }
    const auto offered = vkEnumerate<VkExtensionProperties>(
        "vkEnumerateInstanceExtensionProperties", [](uint32_t* count, VkExtensionProperties* items) {
            return vkEnumerateInstanceExtensionProperties(nullptr, count, items);
        });
    const InstanceExtensionPlan plan = planInstanceExtensions(std::span(sdlExtensions, sdlCount), offered, validation);
    if (!plan.missing.empty()) {
        throw std::runtime_error(std::format("Vulkan instance extensions missing: {}", joined(plan.missing)));
    }
    DILITHIUM_LOG_DEBUG("instance extensions: {}", joined(plan.enable));

    const VkApplicationInfo application{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Dilithium2D",
        .pEngineName = "Dilithium2D",
        .apiVersion = VK_API_VERSION_1_3,
    };
    // A copy of the messenger's create-info rides on the instance's own, so instance creation and destruction are
    // checked too, before and after the real messenger exists.
    const VkDebugUtilsMessengerCreateInfoEXT messengerInfo = messengerCreateInfo(m_validationLog.get());
    const char* const layer = kValidationLayer;
    const VkInstanceCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = validation ? &messengerInfo : nullptr,
        .flags = plan.portabilityEnumeration ? VkInstanceCreateFlags{VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR}
                                             : VkInstanceCreateFlags{0},
        .pApplicationInfo = &application,
        .enabledLayerCount = validation ? 1u : 0u,
        .ppEnabledLayerNames = validation ? &layer : nullptr,
        .enabledExtensionCount = static_cast<uint32_t>(plan.enable.size()),
        .ppEnabledExtensionNames = plan.enable.data(),
    };
    VK_CHECK(vkCreateInstance(&info, nullptr, &m_instance));

    if (!validation) {
        return;
    }
    // The instance exists now, and a throw from here on would skip the destructor: destroy it by hand on the way out.
    try {
        auto create = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT"));
        m_destroyMessenger = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT"));
        if (create == nullptr || m_destroyMessenger == nullptr) {
            throw std::runtime_error("VK_EXT_debug_utils is enabled but its functions are missing");
        }
        VK_CHECK(create(m_instance, &messengerInfo, nullptr, &m_messenger));
    } catch (...) {
        vkDestroyInstance(m_instance, nullptr);
        throw;
    }
}

Instance::~Instance() {
    if (m_messenger != VK_NULL_HANDLE) {
        m_destroyMessenger(m_instance, m_messenger, nullptr);
    }
    vkDestroyInstance(m_instance, nullptr);
    if (m_validationLog) {
        // Reported after vkDestroyInstance, so it counts shutdown too.
        const ValidationLog& log = *m_validationLog;
        if (log.errors + log.warnings == 0) {
            logInfo("validation reported nothing");
        } else {
            logWarning("validation reported {} errors and {} warnings", log.errors, log.warnings);
        }
    }
}

} // namespace dilithium
