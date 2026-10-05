#pragma once

// SDL's Vulkan header defines the two Vulkan handle types it needs by itself, so this file names them without a
// Vulkan header: platform/ includes SDL and nothing of Vulkan's.
#include <SDL3/SDL_vulkan.h>

#include <span>

namespace dilithium {

class Window;

// The one place the window system and Vulkan meet, owned by platform/. The graphics code asks the window for what it
// needs through these and never names the window system; LiquidMetal2D's view hands Metal its layer the same way.

/// The instance extensions the window system needs before it can make a surface. The names live as long as the
/// program. Throws `std::runtime_error` with the window system's message if it cannot say.
[[nodiscard]] std::span<const char* const> windowInstanceExtensions();

/// A Vulkan surface for `window` on `instance`. Throws `std::runtime_error` with the window system's message.
/// Destroy it with `destroyWindowSurface`, before the instance.
[[nodiscard]] VkSurfaceKHR createWindowSurface(const Window& window, VkInstance instance);
void destroyWindowSurface(VkInstance instance, VkSurfaceKHR surface);

} // namespace dilithium
