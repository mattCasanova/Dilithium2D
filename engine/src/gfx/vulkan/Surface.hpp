#pragma once

#include <vulkan/vulkan.h>

namespace dilithium {

class Window;

/// The window's Vulkan surface, made by the window through `platform/WindowSurface`. Destroyed before the instance.
/// Not copyable or movable: RenderCore builds it in place.
class Surface {
public:
    /// Throws `std::runtime_error` with the window system's message if it cannot make the surface.
    Surface(VkInstance instance, const Window& window);
    ~Surface();

    Surface(const Surface&) = delete;
    Surface& operator=(const Surface&) = delete;
    Surface(Surface&&) = delete;
    Surface& operator=(Surface&&) = delete;

    [[nodiscard]] VkSurfaceKHR handle() const { return m_surface; }

private:
    VkInstance m_instance;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
};

} // namespace dilithium
