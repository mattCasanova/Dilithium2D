#pragma once

#include <dilithium/core/NonCopyable.hpp>

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

    [[nodiscard]] PixelSize pixelSize() const;
    [[nodiscard]] bool isMinimized() const;

    /// For `--torture`. Each asks the window system and returns at once; the size or state changes a little later,
    /// through events. A refusal is logged, not thrown.
    void resize(int width, int height);
    void minimize();
    void restore();

    /// For the engine's Vulkan surface only.
    [[nodiscard]] SDL_Window* sdlWindow() const { return m_window.get(); }

private:
    struct Destroy {
        void operator()(SDL_Window* window) const;
    };
    std::unique_ptr<SDL_Window, Destroy> m_window;
};

} // namespace dilithium
