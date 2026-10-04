#pragma once

#include <dilithium/gfx/Color.hpp>
#include <dilithium/gfx/Renderer.hpp>
#include <dilithium/math/Types.hpp>

#include <cstdint>
#include <memory>

namespace dilithium {

class Window;

/// The engine's own `Renderer`, on Vulkan: records what a scene asks for, then draws it into the window each frame.
/// LiquidMetal2D's `DefaultRenderer`. The game builds one from its window and hands both to the `Engine`. Vulkan
/// lives behind the `Impl`; this header names none of it.
class DefaultRenderer final : public Renderer {
public:
    /// Brings up the GPU for `window`. Throws `std::runtime_error` when Vulkan or the validation layer is missing.
    /// The window must outlive the renderer; the `Engine` keeps that order.
    explicit DefaultRenderer(const Window& window);
    ~DefaultRenderer() override;

    DefaultRenderer(const DefaultRenderer&) = delete;
    DefaultRenderer& operator=(const DefaultRenderer&) = delete;
    DefaultRenderer(DefaultRenderer&&) = delete;
    DefaultRenderer& operator=(DefaultRenderer&&) = delete;

    void setClearColor(Color color) override;
    void drawTriangle(Vec2 a, Vec2 b, Vec2 c, Color colorA, Color colorB, Color colorC) override;
    FrameOutcome drawFrame() override;

    /// Validation messages so far: errors plus warnings, 0 with validation off.
    [[nodiscard]] uint32_t problemsReported() const override;

    /// How many times the swapchain was built, including the first; a resize costs one.
    [[nodiscard]] uint32_t swapchainBuilds() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace dilithium
