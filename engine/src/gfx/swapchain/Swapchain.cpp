#include "gfx/swapchain/Swapchain.hpp"

#include "gfx/swapchain/SwapchainChoices.hpp"
#include "gfx/vulkan/VkCheck.hpp"

#include <dilithium/core/Log.hpp>

#include <cstdint>
#include <optional>
#include <stdexcept>

namespace dilithium {

Swapchain::Swapchain(VkPhysicalDevice physical, VkDevice newDevice, VkSurfaceKHR surface,
                     const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D newExtent, VkSwapchainKHR oldSwapchain)
    : device(newDevice), extent(newExtent) {
    const auto formats = vkEnumerate<VkSurfaceFormatKHR>(
        "vkGetPhysicalDeviceSurfaceFormatsKHR", [physical, surface](uint32_t* count, VkSurfaceFormatKHR* items) {
            return vkGetPhysicalDeviceSurfaceFormatsKHR(physical, surface, count, items);
        });
    const auto presentModes = vkEnumerate<VkPresentModeKHR>(
        "vkGetPhysicalDeviceSurfacePresentModesKHR", [physical, surface](uint32_t* count, VkPresentModeKHR* items) {
            return vkGetPhysicalDeviceSurfacePresentModesKHR(physical, surface, count, items);
        });
    const std::optional<VkSurfaceFormatKHR> surfaceFormat = chooseSurfaceFormat(formats);
    const std::optional<VkPresentModeKHR> presentMode = choosePresentMode(presentModes);
    const std::optional<VkCompositeAlphaFlagBitsKHR> compositeAlpha =
        chooseCompositeAlpha(capabilities.supportedCompositeAlpha);
    if (!surfaceFormat || !presentMode || !compositeAlpha) {
        throw std::runtime_error("the window's surface offers no usable format, present mode or composite alpha");
    }
    if (surfaceFormat->format != VK_FORMAT_B8G8R8A8_UNORM ||
        surfaceFormat->colorSpace != VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
        logWarning("no B8G8R8A8_UNORM + sRGB swapchain format; using format {} (colors may differ from LiquidMetal2D)",
                   static_cast<int>(surfaceFormat->format));
    }

    format = surfaceFormat->format;
    const VkSwapchainCreateInfoKHR info{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = chooseImageCount(capabilities),
        .imageFormat = surfaceFormat->format,
        .imageColorSpace = surfaceFormat->colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = *compositeAlpha,
        .presentMode = *presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = oldSwapchain,
    };
    VK_CHECK(vkCreateSwapchainKHR(device, &info, nullptr, &swapchain));

    // From here on a throw would skip the destructor, so clean up by hand on the way out.
    try {
        images = vkEnumerate<VkImage>("vkGetSwapchainImagesKHR", [this](uint32_t* count, VkImage* items) {
            return vkGetSwapchainImagesKHR(device, swapchain, count, items);
        });
        for (VkImage image : images) {
            const VkImageViewCreateInfo viewInfo{
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = image,
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = surfaceFormat->format,
                .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
            };
            VkImageView view = VK_NULL_HANDLE;
            VK_CHECK(vkCreateImageView(device, &viewInfo, nullptr, &view));
            views.push_back(view);

            const VkSemaphoreCreateInfo semaphoreInfo{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
            VkSemaphore semaphore = VK_NULL_HANDLE;
            VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &semaphore));
            renderFinished.push_back(semaphore);
        }
    } catch (...) {
        destroy();
        throw;
    }
    DILITHIUM_LOG_DEBUG("swapchain {}x{}, {} images", extent.width, extent.height, images.size());
}

Swapchain::~Swapchain() {
    destroy();
}

void Swapchain::destroy() {
    for (VkSemaphore semaphore : renderFinished) {
        vkDestroySemaphore(device, semaphore, nullptr);
    }
    for (VkImageView view : views) {
        vkDestroyImageView(device, view, nullptr);
    }
    vkDestroySwapchainKHR(device, swapchain, nullptr);
    renderFinished.clear();
    views.clear();
    images.clear();
    swapchain = VK_NULL_HANDLE;
}

} // namespace dilithium
