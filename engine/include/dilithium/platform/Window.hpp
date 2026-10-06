#pragma once

#include <dilithium/platform/AppState.hpp>
#include <dilithium/utilities/NonMovable.hpp>

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
/// comes from `getPixelSize()`, never the size in points. The window system behind it is in the `.cpp` only.
class Window : NonMovable {
public:
    /// Throws `std::runtime_error` with the window system's message if the window cannot be made.
    explicit Window(const WindowConfig& config);
    ~Window();

    [[nodiscard]] PixelSize getPixelSize() const;
    [[nodiscard]] bool isMinimized() const;

    /// Whether the player can see and use the window right now, read from the window system's own flags: hidden or
    /// minimized is `Background`; otherwise focus decides between `Active` and `Inactive`. The engine asks after
    /// every window event and tells the game when the answer changes.
    [[nodiscard]] AppState getAppState() const;

    /// Each asks the window system and returns at once; the size or state changes a little later, through events.
    /// A refusal is logged, not thrown. The stress run under `tests/stress` uses them; a game may too.
    void resize(int width, int height);
    void minimize();
    void restore();

    /// The window system's handle, for the engine's own platform code (the Vulkan surface seam). Defined in a
    /// private header, so a game sees only an incomplete type here.
    struct Impl;
    [[nodiscard]] const Impl& getImpl() const { return *impl; }

private:
    std::unique_ptr<Impl> impl;
};

} // namespace dilithium
