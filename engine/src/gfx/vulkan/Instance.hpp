#pragma once

#include <dilithium/utilities/NonMovable.hpp>

#include <vulkan/vulkan.h>

#include <cstdint>
#include <memory>

namespace dilithium {

/// What the validation layer reported. On the heap, so the pointer the messenger holds stays valid for as long as
/// Vulkan may call it.
struct ValidationLog {
    uint32_t warnings = 0;
    uint32_t errors = 0;
};

/// The Vulkan instance. In debug builds it runs the Khronos validation layer with a debug messenger, unless
/// `DILITHIUM_NO_VALIDATION=1`; a validation error then aborts at the call that caused it. Not copyable or movable:
/// RenderCore builds it in place.
class Instance : NonMovable {
public:
    /// Throws `std::runtime_error` when the loader is older than 1.3, an extension the window needs is missing, or
    /// (with validation on) the validation layer is not installed.
    Instance();
    ~Instance();

    [[nodiscard]] VkInstance handle() const { return instance; }

    /// Errors plus warnings the validation layer has reported so far; 0 with validation off.
    [[nodiscard]] uint32_t getValidationMessages() const {
        return validationLog ? validationLog->errors + validationLog->warnings : 0;
    }

private:
    std::unique_ptr<ValidationLog> validationLog; ///< null when validation is off; outlives both handles
    VkInstance instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
    PFN_vkDestroyDebugUtilsMessengerEXT destroyMessenger = nullptr;
};

} // namespace dilithium
