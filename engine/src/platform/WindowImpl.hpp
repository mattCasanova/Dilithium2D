#pragma once

#include <dilithium/platform/Window.hpp>

struct SDL_Window;

namespace dilithium {

/// What the public `Window` hides: the SDL window. Included by `Window.cpp` and by the renderer that needs the
/// handle for its surface; nothing a game includes reaches it.
struct Window::Impl {
    SDL_Window* handle = nullptr;
};

} // namespace dilithium
