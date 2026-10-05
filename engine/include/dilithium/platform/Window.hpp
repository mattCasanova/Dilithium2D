#pragma once

#include <cstdint>
#include <memory>
#include <string>

namespace dilithium {

/// The window a program starts with. Sizes are in points, not pixels: on a high-density display the window has
/// more pixels than this.
struct WindowConfig {
    static constexpr int kDefaultWidth = 1280;
    static constexpr int kDefaultHeight = 720;

    std::string title = "Dilithium2D";
    int width = kDefaultWidth;
    int height = kDefaultHeight;
};

struct PixelSize {
    uint32_t width = 0;
    uint32_t height = 0;
};

/// The program's one window: resizable, full pixel density, ready for a renderer to draw into. LiquidMetal2D's
/// `parentView`. The game builds it and hands it to the `Engine`, which owns it. Every size that reaches the GPU
/// comes from `pixelSize()`, never the size in points. The window system behind it is in the `.cpp` only.
class Window {
public:
    /// Throws `std::runtime_error` with the window system's message if the window cannot be made.
    explicit Window(const WindowConfig& config);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(Window&&) = delete;

    [[nodiscard]] PixelSize pixelSize() const;
    [[nodiscard]] bool isMinimized() const;

    /// Each asks the window system and returns at once; the size or state changes a little later, through events.
    /// A refusal is logged, not thrown. The stress run under `tests/stress` uses them; a game may too.
    void resize(int width, int height);
    void minimize();
    void restore();

    /// The window system's handle, for the engine's own platform code (the Vulkan surface seam). Defined in a
    /// private header, so a game sees only an incomplete type here.
    struct Impl;
    [[nodiscard]] const Impl& impl() const { return *m_impl; }

private:
    std::unique_ptr<Impl> m_impl;
};

} // namespace dilithium
