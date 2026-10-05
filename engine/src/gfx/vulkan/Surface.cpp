#include "gfx/vulkan/Surface.hpp"

#include "platform/WindowSurface.hpp"

namespace dilithium {

Surface::Surface(VkInstance newInstance, const Window& window)
    : instance(newInstance), surface(createWindowSurface(window, newInstance)) {}

Surface::~Surface() {
    destroyWindowSurface(instance, surface);
}

} // namespace dilithium
