#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>
#include <optional>
#include <span>

// Pure choices for building a swapchain, over the plain structs the surface reports. No GPU calls, so they are unit
// tested without one. `std::nullopt` means the surface offers nothing usable; the caller makes that a fatal error.

namespace dilithium {

/// `B8G8R8A8_UNORM` + sRGB nonlinear, matching LiquidMetal2D's `.bgra8Unorm` layer, so the same color values look the
/// same in both engines. Without that pair, the first format offered.
std::optional<VkSurfaceFormatKHR> chooseSurfaceFormat(std::span<const VkSurfaceFormatKHR> offered);

/// FIFO: vsync, and our frame limiter (no busy-wait). The spec guarantees it, so `std::nullopt` means a broken driver.
std::optional<VkPresentModeKHR> choosePresentMode(std::span<const VkPresentModeKHR> offered);

/// The surface's fixed `currentExtent` when it has one. Otherwise the window's pixel size, clamped to the surface's
/// limits. A window with a zero side (minimized) gives `{0, 0}`: the caller skips frames until it grows again.
VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D windowPixels);

/// One more than the minimum, so we rarely wait on the driver for an image. Capped by `maxImageCount` unless that is 0,
/// which means no limit.
uint32_t chooseImageCount(const VkSurfaceCapabilitiesKHR& capabilities);

/// OPAQUE when supported, else the lowest supported bit; `std::nullopt` if none is (the spec promises one).
std::optional<VkCompositeAlphaFlagBitsKHR> chooseCompositeAlpha(VkCompositeAlphaFlagsKHR supported);

} // namespace dilithium
