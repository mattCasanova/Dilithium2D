#pragma once

#include <dilithium/utilities/NonMovable.hpp>

#include <vulkan/vulkan.h>

namespace dilithium {

class Window;

/// The window's Vulkan surface, made by the window through `platform/WindowSurface`. Destroyed before the instance.
/// Not copyable or movable: RenderCore builds it in place.
class Surface : NonMovable {
public:
    /// Throws `std::runtime_error` with the window system's message if it cannot make the surface.
    Surface(VkInstance newInstance, const Window& window);
    ~Surface();

    [[nodiscard]] VkSurfaceKHR handle() const { return surface; }

private:
    VkInstance instance;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
};

} // namespace dilithium
