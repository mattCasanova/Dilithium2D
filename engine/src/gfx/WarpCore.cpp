#include "gfx/WarpCore.hpp"

#include "gfx/VkCheck.hpp"
#include "platform/Window.hpp"

#include <dilithium/core/Log.hpp>

namespace dilithium {

WarpCore::WarpCore(const Window& window)
    : m_surface(m_instance.handle(), window.sdlWindow()), m_device(m_instance.handle(), m_surface.handle()),
      m_allocator(m_instance.handle(), m_device.physical(), m_device.handle()) {}

WarpCore::~WarpCore() {
    // The GPU may still be using what the members are about to destroy. A destructor must not throw, so a failure
    // here is logged rather than sent through KHAAAN.
    const VkResult idle = vkDeviceWaitIdle(m_device.handle());
    if (idle != VK_SUCCESS) {
        logError("vkDeviceWaitIdle at shutdown returned {}", describeVkResult(idle));
    }
}

} // namespace dilithium
