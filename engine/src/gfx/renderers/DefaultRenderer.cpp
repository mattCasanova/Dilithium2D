#include "gfx/renderers/RenderCore.hpp"

#include <dilithium/core/Assert.hpp>
#include <dilithium/core/Log.hpp>
#include <dilithium/gfx/Color.hpp>
#include <dilithium/gfx/Renderer.hpp>
#include <dilithium/gfx/renderers/DefaultRenderer.hpp>
#include <dilithium/math/Types.hpp>

#include <cstdint>
#include <memory>
#include <vector>

namespace dilithium {
namespace {

/// One corner of a triangle, as the color shader's vertex input will want it (D2 Phase 4 pins the layout).
struct ColorVertex {
    Vec2 position;
    Color color;
};

} // namespace

struct DefaultRenderer::Impl {
    explicit Impl(const Window& newWindow) : window(newWindow), core(newWindow) {}

    const Window& window;
    RenderCore core;
    Color clearColor;                   ///< what the current frame starts from; black until a scene says otherwise
    std::vector<ColorVertex> triangles; ///< what the scene asked to draw this frame, three vertices each
};

DefaultRenderer::DefaultRenderer(const Window& window) : m_impl(std::make_unique<Impl>(window)) {}

// NOLINTNEXTLINE(bugprone-exception-escape): it logs; std::format's bad_alloc at shutdown may end the program, rightly
DefaultRenderer::~DefaultRenderer() {
    logInfo("renderer: {} swapchain builds", m_impl->core.swapchainBuilds());
}

void DefaultRenderer::setClearColor(Color color) {
    m_impl->clearColor = color;
}

void DefaultRenderer::drawTriangle(Vec2 a, Vec2 b, Vec2 c, Color colorA, Color colorB, Color colorC) {
    m_impl->triangles.push_back({a, colorA});
    m_impl->triangles.push_back({b, colorB});
    m_impl->triangles.push_back({c, colorC});
}

FrameOutcome DefaultRenderer::drawFrame() {
    // Whatever happens, the recorded list belongs to this frame alone.
    std::vector<ColorVertex>& triangles = m_impl->triangles;
    const auto dropRecording = [&triangles] { triangles.clear(); };

    switch (m_impl->core.beginFrame(m_impl->window, m_impl->clearColor)) {
    case FrameBegin::Ready:
        // The triangles are drawn here from Phase 5; until then the frame is the clear alone.
        m_impl->core.endFrame();
        dropRecording();
        return FrameOutcome::Presented;
    case FrameBegin::Skipped:
        dropRecording();
        return FrameOutcome::Skipped;
    case FrameBegin::Idle:
        dropRecording();
        return FrameOutcome::Idle;
    }
    DILITHIUM_UNREACHABLE("unknown FrameBegin");
}

uint32_t DefaultRenderer::problemsReported() const {
    return m_impl->core.validationMessages();
}

uint32_t DefaultRenderer::swapchainBuilds() const {
    return m_impl->core.swapchainBuilds();
}

} // namespace dilithium
