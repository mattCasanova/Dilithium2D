#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>
#include <vector>

namespace dilithium {

/// The images the screen shows, rebuilt whenever the window changes size. Each image has its own view and its own
/// "rendering finished" semaphore: with 3 images and 2 frames in flight, a semaphore per frame slot could be
/// signaled again while a present still waits on it. Not copyable or movable: WarpCore swaps in a new one, and the
/// old one lives until the new one exists.
class Swapchain {
public:
    /// `oldSwapchain` is the one being replaced, or `VK_NULL_HANDLE`. Throws `std::runtime_error` when the surface
    /// offers no usable format, present mode or composite alpha.
    Swapchain(VkPhysicalDevice physical, VkDevice device, VkSurfaceKHR surface,
              const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D extent, VkSwapchainKHR oldSwapchain);
    ~Swapchain();

    Swapchain(const Swapchain&) = delete;
    Swapchain& operator=(const Swapchain&) = delete;
    Swapchain(Swapchain&&) = delete;
    Swapchain& operator=(Swapchain&&) = delete;

    VkSwapchainKHR handle() const { return m_swapchain; }
    VkExtent2D extent() const { return m_extent; }
    VkImage image(uint32_t index) const { return m_images.at(index); }
    VkImageView view(uint32_t index) const { return m_views.at(index); }
    VkSemaphore renderFinished(uint32_t index) const { return m_renderFinished.at(index); }

private:
    void destroy();

    VkDevice m_device;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    VkExtent2D m_extent{};
    std::vector<VkImage> m_images; ///< owned by the swapchain, not by us
    std::vector<VkImageView> m_views;
    std::vector<VkSemaphore> m_renderFinished;
};

} // namespace dilithium
