#include "gfx/Surface.hpp"

#include <SDL3/SDL_vulkan.h>

#include <format>
#include <stdexcept>

namespace dilithium {

Surface::Surface(VkInstance instance, SDL_Window* window) : m_instance(instance) {
    if (!SDL_Vulkan_CreateSurface(window, instance, nullptr, &m_surface)) {
        throw std::runtime_error(std::format("SDL_Vulkan_CreateSurface failed: {}", SDL_GetError()));
    }
}

Surface::~Surface() {
    SDL_Vulkan_DestroySurface(m_instance, m_surface, nullptr);
}

} // namespace dilithium
