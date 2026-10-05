#include "platform/WindowSurface.hpp"

#include "platform/WindowImpl.hpp"

#include <dilithium/platform/Window.hpp>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include <format>
#include <span>
#include <stdexcept>

namespace dilithium {

std::span<const char* const> windowInstanceExtensions() {
    Uint32 count = 0;
    const char* const* names = SDL_Vulkan_GetInstanceExtensions(&count);
    if (names == nullptr) {
        throw std::runtime_error(std::format("SDL_Vulkan_GetInstanceExtensions failed: {}", SDL_GetError()));
    }
    return {names, count};
}

VkSurfaceKHR createWindowSurface(const Window& window, VkInstance instance) {
    VkSurfaceKHR surface{};
    if (!SDL_Vulkan_CreateSurface(window.getImpl().handle, instance, nullptr, &surface)) {
        throw std::runtime_error(std::format("SDL_Vulkan_CreateSurface failed: {}", SDL_GetError()));
    }
    return surface;
}

void destroyWindowSurface(VkInstance instance, VkSurfaceKHR surface) {
    SDL_Vulkan_DestroySurface(instance, surface, nullptr);
}

} // namespace dilithium
