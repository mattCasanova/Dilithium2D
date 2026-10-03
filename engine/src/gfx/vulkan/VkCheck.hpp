#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>
#include <source_location>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace dilithium {

/// Thrown by `VK_CHECK` in release builds (debug builds abort at the failing line instead).
class VulkanError : public std::runtime_error {
public:
    VulkanError(const std::string& message, VkResult result) : std::runtime_error(message), m_result(result) {}

    [[nodiscard]] VkResult result() const { return m_result; }

private:
    VkResult m_result;
};

/// "VK_ERROR_DEVICE_LOST (-4)". A value newer than this engine's list still prints, as "VkResult (1000299000)".
std::string describeVkResult(VkResult result);

namespace detail {

/// The failure path of `VK_CHECK`: reports the call, its result and the caller's location, then aborts (debug) or
/// throws `VulkanError` (release). `VK_ERROR_DEVICE_LOST` is reported as "device lost".
[[noreturn]] void vkCallFailed(VkResult result, std::string_view call, const std::source_location& where);

/// Behind `VK_CHECK`. Inline, so a successful call costs one comparison.
inline void checkVk(VkResult result, std::string_view call,
                    const std::source_location& where = std::source_location::current()) {
    if (result != VK_SUCCESS) [[unlikely]] {
        vkCallFailed(result, call, where);
    }
}

} // namespace detail

/// Vulkan's "ask for the count, then fill" idiom, retried while the list grows between the two calls
/// (`VK_INCOMPLETE`). `call` names the Vulkan function for the error report.
template <typename T, typename Query>
std::vector<T> vkEnumerate(std::string_view call, const Query& query,
                           const std::source_location& where = std::source_location::current()) {
    std::vector<T> items;
    VkResult result = VK_INCOMPLETE;
    while (result == VK_INCOMPLETE) {
        uint32_t count = 0;
        detail::checkVk(query(&count, nullptr), call, where);
        items.resize(count);
        result = query(&count, items.data());
        items.resize(count);
    }
    detail::checkVk(result, call, where);
    return items;
}

} // namespace dilithium

/// Checks a Vulkan call that must return `VK_SUCCESS`: on anything else, logs the call, the result's name, file and
/// line, then aborts in debug builds or throws `dilithium::VulkanError` in release. Results that are part of normal
/// flow (`vkAcquireNextImageKHR`, `vkQueuePresentKHR`) are handled by hand, never through `VK_CHECK`.
#define VK_CHECK(call) ::dilithium::detail::checkVk((call), #call)
