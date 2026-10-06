#pragma once

#include <dilithium/utilities/NonMovable.hpp>

#include <vulkan/vulkan.h>

#include <cstdint>
#include <vector>

namespace dilithium {

/// The images the screen shows, rebuilt whenever the window changes size. Each image has its own view and its own
/// "rendering finished" semaphore: with 3 images and 2 frames in flight, a semaphore per frame slot could be
/// signaled again while a present still waits on it. Not copyable or movable: RenderCore swaps in a new one, and the
/// old one lives until the new one exists.
class Swapchain : NonMovable {
public:
    /// `oldSwapchain` is the one being replaced, or `VK_NULL_HANDLE`. Throws `std::runtime_error` when the surface
    /// offers no usable format, present mode or composite alpha.
    Swapchain(VkPhysicalDevice physical, VkDevice newDevice, VkSurfaceKHR surface,
              const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D newExtent, VkSwapchainKHR oldSwapchain);
    ~Swapchain();

    [[nodiscard]] VkSwapchainKHR handle() const { return swapchain; }
    [[nodiscard]] VkExtent2D getExtent() const { return extent; }
    [[nodiscard]] VkFormat getFormat() const { return format; }
    [[nodiscard]] VkImage getImage(uint32_t index) const { return images.at(index); }
    [[nodiscard]] VkImageView getView(uint32_t index) const { return views.at(index); }
    [[nodiscard]] VkSemaphore getRenderFinished(uint32_t index) const { return renderFinished.at(index); }

private:
    void destroy();

    VkDevice device;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkExtent2D extent{};
    VkFormat format = VK_FORMAT_UNDEFINED;
    std::vector<VkImage> images; ///< owned by the swapchain, not by us
    std::vector<VkImageView> views;
    std::vector<VkSemaphore> renderFinished;
};

} // namespace dilithium
