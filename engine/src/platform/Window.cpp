#include "platform/Window.hpp"

#include <dilithium/core/Assert.hpp>
#include <dilithium/core/Log.hpp>

#include <SDL3/SDL.h>

#include <cstdint>
#include <format>
#include <stdexcept>
#include <string>

namespace dilithium {

void Window::Destroy::operator()(SDL_Window* window) const {
    SDL_DestroyWindow(window);
}

Window::Window(const std::string& title, int width, int height)
    : m_window(SDL_CreateWindow(title.c_str(), width, height,
                                SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY)) {
    if (!m_window) {
        throw std::runtime_error(std::format("SDL_CreateWindow failed: {}", SDL_GetError()));
    }
}

PixelSize Window::pixelSize() const {
    int width = 0;
    int height = 0;
    if (!SDL_GetWindowSizeInPixels(m_window.get(), &width, &height)) {
        DILITHIUM_UNREACHABLE(std::format("SDL_GetWindowSizeInPixels failed on a live window: {}", SDL_GetError()));
    }
    DILITHIUM_ASSERT(width >= 0 && height >= 0, "SDL reported a negative window size");
    return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
}

bool Window::isMinimized() const {
    return (SDL_GetWindowFlags(m_window.get()) & SDL_WINDOW_MINIMIZED) != 0;
}

void Window::resize(int width, int height) {
    if (!SDL_SetWindowSize(m_window.get(), width, height)) {
        logWarning("SDL_SetWindowSize({}, {}) refused: {}", width, height, SDL_GetError());
    }
}

void Window::minimize() {
    if (!SDL_MinimizeWindow(m_window.get())) {
        logWarning("SDL_MinimizeWindow refused: {}", SDL_GetError());
    }
}

void Window::restore() {
    if (!SDL_RestoreWindow(m_window.get())) {
        logWarning("SDL_RestoreWindow refused: {}", SDL_GetError());
    }
}

} // namespace dilithium
