#include "gfx/SwapchainChoices.hpp"

#include <dilithium/Assert.hpp>

#include <algorithm>
#include <bit>
#include <limits>

namespace dilithium {

std::optional<VkSurfaceFormatKHR> chooseSurfaceFormat(std::span<const VkSurfaceFormatKHR> offered) {
    if (offered.empty()) {
        return std::nullopt;
    }
    const auto preferred = std::ranges::find_if(offered, [](const VkSurfaceFormatKHR& candidate) {
        return candidate.format == VK_FORMAT_B8G8R8A8_UNORM &&
               candidate.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    });
    return preferred != offered.end() ? *preferred : offered.front();
}

std::optional<VkPresentModeKHR> choosePresentMode(std::span<const VkPresentModeKHR> offered) {
    if (std::ranges::find(offered, VK_PRESENT_MODE_FIFO_KHR) == offered.end()) {
        return std::nullopt;
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D windowPixels) {
    // UINT32_MAX is the spec's "the swapchain decides" value; anything else is the size we must use.
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }
    if (windowPixels.width == 0 || windowPixels.height == 0) {
        return {0, 0};
    }

    const VkExtent2D& low = capabilities.minImageExtent;
    const VkExtent2D& high = capabilities.maxImageExtent;
    LOGICAL(low.width <= high.width && low.height <= high.height, "surface reports min extent above max extent");
    return {std::clamp(windowPixels.width, low.width, high.width),
            std::clamp(windowPixels.height, low.height, high.height)};
}

uint32_t chooseImageCount(const VkSurfaceCapabilitiesKHR& capabilities) {
    const uint32_t wanted = capabilities.minImageCount + 1;
    const bool capped = capabilities.maxImageCount != 0;
    return capped ? std::min(wanted, capabilities.maxImageCount) : wanted;
}

std::optional<VkCompositeAlphaFlagBitsKHR> chooseCompositeAlpha(VkCompositeAlphaFlagsKHR supported) {
    if (supported == 0) {
        return std::nullopt;
    }
    if ((supported & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) != 0) {
        return VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    }
    return static_cast<VkCompositeAlphaFlagBitsKHR>(1u << std::countr_zero(supported));
}

} // namespace dilithium
