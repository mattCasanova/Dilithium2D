#pragma once

#include <dilithium/gfx/Color.hpp>

namespace dilithium {

/// How one frame went, from `Renderer::drawFrame`.
enum class FrameOutcome {
    Presented, ///< drawn and handed to the screen; the present paced the frame
    Skipped,   ///< the swapchain went out of date; it is rebuilt at the start of the next frame, so call again now
    Idle,      ///< the window is minimized or has no area; nothing paced the frame, so the caller should sleep
};

/// What draws the frame, as LiquidMetal2D's `Renderer` protocol: one interface with both halves. Scenes call the
/// recording half during `Scene::draw()`; the engine calls `drawFrame()` once per frame after it. `DefaultRenderer`
/// is the engine's own implementation; a game, a tool or a test passes in another. Nothing here names Vulkan.
class Renderer {
public:
    Renderer() = default;
    virtual ~Renderer() = default;

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    // --- what a scene calls, from draw()

    /// The color the frame starts from. D2 adds drawing on top of it.
    virtual void setClearColor(Color color) = 0;

    // --- what the engine calls

    /// Draws what the scene recorded and presents it. The engine calls it once per frame; a scene never does.
    virtual FrameOutcome drawFrame() = 0;
};

} // namespace dilithium
