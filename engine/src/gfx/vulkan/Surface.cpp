#include "gfx/vulkan/Surface.hpp"

#include "platform/WindowSurface.hpp"

namespace dilithium {

Surface::Surface(VkInstance instance, const Window& window)
    : m_instance(instance), m_surface(createWindowSurface(window, instance)) {}

Surface::~Surface() {
    destroyWindowSurface(m_instance, m_surface);
}

} // namespace dilithium
