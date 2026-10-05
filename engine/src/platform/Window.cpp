#include "platform/WindowImpl.hpp"

#include <dilithium/platform/AppState.hpp>
#include <dilithium/platform/Window.hpp>
#include <dilithium/utilities/Assert.hpp>
#include <dilithium/utilities/Log.hpp>

#include <SDL3/SDL.h>

#include <cstdint>
#include <format>
#include <memory>
#include <stdexcept>

namespace dilithium {

Window::Window(const WindowConfig& config) : impl(std::make_unique<Impl>()) {
    impl->handle = SDL_CreateWindow(config.title.c_str(), config.width, config.height,
                                    SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (impl->handle == nullptr) {
        throw std::runtime_error(std::format("SDL_CreateWindow failed: {}", SDL_GetError()));
    }
    const PixelSize pixels = getPixelSize();
    logInfo("window '{}': {}x{} pixels", config.title, pixels.width, pixels.height);
}

Window::~Window() {
    SDL_DestroyWindow(impl->handle);
}

PixelSize Window::getPixelSize() const {
    int width = 0;
    int height = 0;
    if (!SDL_GetWindowSizeInPixels(impl->handle, &width, &height)) {
        DILITHIUM_UNREACHABLE(std::format("SDL_GetWindowSizeInPixels failed on a live window: {}", SDL_GetError()));
    }
    DILITHIUM_ASSERT(width >= 0 && height >= 0, "SDL reported a negative window size");
    return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
}

bool Window::isMinimized() const {
    return (SDL_GetWindowFlags(impl->handle) & SDL_WINDOW_MINIMIZED) != 0;
}

AppState Window::getAppState() const {
    const SDL_WindowFlags flags = SDL_GetWindowFlags(impl->handle);
    if ((flags & (SDL_WINDOW_HIDDEN | SDL_WINDOW_MINIMIZED)) != 0) {
        return AppState::Background;
    }
    return (flags & SDL_WINDOW_INPUT_FOCUS) != 0 ? AppState::Active : AppState::Inactive;
}

void Window::resize(int width, int height) {
    if (!SDL_SetWindowSize(impl->handle, width, height)) {
        logWarning("SDL_SetWindowSize({}, {}) refused: {}", width, height, SDL_GetError());
    }
}

void Window::minimize() {
    if (!SDL_MinimizeWindow(impl->handle)) {
        logWarning("SDL_MinimizeWindow refused: {}", SDL_GetError());
    }
}

void Window::restore() {
    if (!SDL_RestoreWindow(impl->handle)) {
        logWarning("SDL_RestoreWindow refused: {}", SDL_GetError());
    }
}

} // namespace dilithium
