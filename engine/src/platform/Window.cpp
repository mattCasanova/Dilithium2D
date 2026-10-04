#include "platform/WindowImpl.hpp"

#include <dilithium/core/Assert.hpp>
#include <dilithium/core/Log.hpp>
#include <dilithium/platform/Window.hpp>

#include <SDL3/SDL.h>

#include <cstdint>
#include <format>
#include <memory>
#include <stdexcept>

namespace dilithium {

Window::Window(const WindowConfig& config) : m_impl(std::make_unique<Impl>()) {
    m_impl->handle = SDL_CreateWindow(config.title.c_str(), config.width, config.height,
                                      SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (m_impl->handle == nullptr) {
        throw std::runtime_error(std::format("SDL_CreateWindow failed: {}", SDL_GetError()));
    }
    const PixelSize pixels = pixelSize();
    logInfo("window '{}': {}x{} pixels", config.title, pixels.width, pixels.height);
}

Window::~Window() {
    SDL_DestroyWindow(m_impl->handle);
}

PixelSize Window::pixelSize() const {
    int width = 0;
    int height = 0;
    if (!SDL_GetWindowSizeInPixels(m_impl->handle, &width, &height)) {
        DILITHIUM_UNREACHABLE(std::format("SDL_GetWindowSizeInPixels failed on a live window: {}", SDL_GetError()));
    }
    DILITHIUM_ASSERT(width >= 0 && height >= 0, "SDL reported a negative window size");
    return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
}

bool Window::isMinimized() const {
    return (SDL_GetWindowFlags(m_impl->handle) & SDL_WINDOW_MINIMIZED) != 0;
}

void Window::resize(int width, int height) {
    if (!SDL_SetWindowSize(m_impl->handle, width, height)) {
        logWarning("SDL_SetWindowSize({}, {}) refused: {}", width, height, SDL_GetError());
    }
}

void Window::minimize() {
    if (!SDL_MinimizeWindow(m_impl->handle)) {
        logWarning("SDL_MinimizeWindow refused: {}", SDL_GetError());
    }
}

void Window::restore() {
    if (!SDL_RestoreWindow(m_impl->handle)) {
        logWarning("SDL_RestoreWindow refused: {}", SDL_GetError());
    }
}

} // namespace dilithium
