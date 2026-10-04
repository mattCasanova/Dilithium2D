#include "gfx/renderers/RenderCore.hpp"

#include <dilithium/core/Log.hpp>
#include <dilithium/gfx/Color.hpp>
#include <dilithium/gfx/Renderer.hpp>
#include <dilithium/gfx/renderers/DefaultRenderer.hpp>

#include <cstdint>
#include <memory>

namespace dilithium {

struct DefaultRenderer::Impl {
    explicit Impl(const Window& newWindow) : window(newWindow), core(newWindow) {}

    const Window& window;
    RenderCore core;
    Color clearColor; ///< what the current frame starts from; black until a scene says otherwise
};

DefaultRenderer::DefaultRenderer(const Window& window) : m_impl(std::make_unique<Impl>(window)) {}

// NOLINTNEXTLINE(bugprone-exception-escape): it logs; std::format's bad_alloc at shutdown may end the program, rightly
DefaultRenderer::~DefaultRenderer() {
    logInfo("renderer: {} swapchain builds", m_impl->core.swapchainBuilds());
}

void DefaultRenderer::setClearColor(Color color) {
    m_impl->clearColor = color;
}

FrameOutcome DefaultRenderer::drawFrame() {
    return m_impl->core.drawFrame(m_impl->window, m_impl->clearColor);
}

uint32_t DefaultRenderer::problemsReported() const {
    return m_impl->core.validationMessages();
}

uint32_t DefaultRenderer::swapchainBuilds() const {
    return m_impl->core.swapchainBuilds();
}

} // namespace dilithium
