#pragma once

#include <dilithium/NonCopyable.hpp>

#include <cstdint>
#include <memory>
#include <string>

struct SDL_Window;

namespace dilithium {

struct PixelSize {
    uint32_t width = 0;
    uint32_t height = 0;
};

/// The program's one SDL window: resizable, full pixel density, ready for a Vulkan surface. Every size that reaches
/// Vulkan comes from `pixelSize()`, never the size in points.
class Window : NonCopyable {
public:
    /// Throws `std::runtime_error` with SDL's message if the window cannot be made.
    Window(const std::string& title, int width, int height);

    PixelSize pixelSize() const;
    bool isMinimized() const;

private:
    struct Destroy {
        void operator()(SDL_Window* window) const;
    };
    std::unique_ptr<SDL_Window, Destroy> m_window;
};

} // namespace dilithium
