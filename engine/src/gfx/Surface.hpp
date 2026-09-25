#pragma once

#include <vulkan/vulkan.h>

struct SDL_Window;

namespace dilithium {

/// The window's Vulkan surface, made by SDL. Destroyed before the instance. Not copyable or movable: WarpCore builds
/// it in place.
class Surface {
public:
    /// Throws `std::runtime_error` with SDL's message if SDL cannot make the surface.
    Surface(VkInstance instance, SDL_Window* window);
    ~Surface();

    Surface(const Surface&) = delete;
    Surface& operator=(const Surface&) = delete;
    Surface(Surface&&) = delete;
    Surface& operator=(Surface&&) = delete;

    VkSurfaceKHR handle() const { return m_surface; }

private:
    VkInstance m_instance;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
};

} // namespace dilithium
