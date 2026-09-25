#include "gfx/Swapchain.hpp"

#include "gfx/SwapchainChoices.hpp"
#include "gfx/VkCheck.hpp"

#include <dilithium/core/Log.hpp>

#include <optional>
#include <stdexcept>

namespace dilithium {

Swapchain::Swapchain(VkPhysicalDevice physical, VkDevice device, VkSurfaceKHR surface,
                     const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D extent, VkSwapchainKHR oldSwapchain)
    : m_device(device), m_extent(extent) {
    const auto formats = vkEnumerate<VkSurfaceFormatKHR>(
        "vkGetPhysicalDeviceSurfaceFormatsKHR", [physical, surface](uint32_t* count, VkSurfaceFormatKHR* items) {
            return vkGetPhysicalDeviceSurfaceFormatsKHR(physical, surface, count, items);
        });
    const auto presentModes = vkEnumerate<VkPresentModeKHR>(
        "vkGetPhysicalDeviceSurfacePresentModesKHR", [physical, surface](uint32_t* count, VkPresentModeKHR* items) {
            return vkGetPhysicalDeviceSurfacePresentModesKHR(physical, surface, count, items);
        });
    const std::optional<VkSurfaceFormatKHR> format = chooseSurfaceFormat(formats);
    const std::optional<VkPresentModeKHR> presentMode = choosePresentMode(presentModes);
    const std::optional<VkCompositeAlphaFlagBitsKHR> compositeAlpha =
        chooseCompositeAlpha(capabilities.supportedCompositeAlpha);
    if (!format || !presentMode || !compositeAlpha) {
        throw std::runtime_error("the window's surface offers no usable format, present mode or composite alpha");
    }
    if (format->format != VK_FORMAT_B8G8R8A8_UNORM || format->colorSpace != VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
        logWarning("no B8G8R8A8_UNORM + sRGB swapchain format; using format {} (colors may differ from LiquidMetal2D)",
                   static_cast<int>(format->format));
    }

    const VkSwapchainCreateInfoKHR info{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = chooseImageCount(capabilities),
        .imageFormat = format->format,
        .imageColorSpace = format->colorSpace,
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
    KHAAAN(vkCreateSwapchainKHR(device, &info, nullptr, &m_swapchain));

    // From here on a throw would skip the destructor, so clean up by hand on the way out.
    try {
        m_images = vkEnumerate<VkImage>("vkGetSwapchainImagesKHR", [this](uint32_t* count, VkImage* items) {
            return vkGetSwapchainImagesKHR(m_device, m_swapchain, count, items);
        });
        for (VkImage image : m_images) {
            const VkImageViewCreateInfo viewInfo{
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = image,
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = format->format,
                .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
            };
            VkImageView view = VK_NULL_HANDLE;
            KHAAAN(vkCreateImageView(device, &viewInfo, nullptr, &view));
            m_views.push_back(view);

            const VkSemaphoreCreateInfo semaphoreInfo{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
            VkSemaphore renderFinished = VK_NULL_HANDLE;
            KHAAAN(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinished));
            m_renderFinished.push_back(renderFinished);
        }
    } catch (...) {
        destroy();
        throw;
    }
    CAPTAINS_LOG("swapchain {}x{}, {} images", extent.width, extent.height, m_images.size());
}

Swapchain::~Swapchain() {
    destroy();
}

void Swapchain::destroy() {
    for (VkSemaphore semaphore : m_renderFinished) {
        vkDestroySemaphore(m_device, semaphore, nullptr);
    }
    for (VkImageView view : m_views) {
        vkDestroyImageView(m_device, view, nullptr);
    }
    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
    m_renderFinished.clear();
    m_views.clear();
    m_images.clear();
    m_swapchain = VK_NULL_HANDLE;
}

} // namespace dilithium
