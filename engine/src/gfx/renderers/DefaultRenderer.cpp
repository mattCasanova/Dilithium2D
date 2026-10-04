#include "gfx/buffers/Buffer.hpp"
#include "gfx/renderers/ColorVertex.hpp"
#include "gfx/renderers/RenderCore.hpp"
#include "gfx/shaders/ColorPipeline.hpp"
#include "gfx/swapchain/FramesInFlight.hpp"

#include <dilithium/core/Assert.hpp>
#include <dilithium/core/Log.hpp>
#include <dilithium/gfx/Color.hpp>
#include <dilithium/gfx/Renderer.hpp>
#include <dilithium/gfx/renderers/DefaultRenderer.hpp>
#include <dilithium/math/Types.hpp>

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

namespace dilithium {
namespace {

/// One vertex buffer per frame slot (LiquidMetal2D's `BufferProvider`): the slot's fence guarantees the GPU is done
/// reading a slot's buffer before the CPU writes it again.
std::array<Buffer, kFramesInFlight> makeVertexBuffers(VmaAllocator allocator) {
    constexpr VkDeviceSize kBytes = sizeof(ColorVertex) * kMaxColorVertices;
    std::array<Buffer, kFramesInFlight> buffers;
    for (Buffer& buffer : buffers) {
        buffer = Buffer::hostVisible(allocator, kBytes, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    }
    return buffers;
}

} // namespace

struct DefaultRenderer::Impl {
    explicit Impl(const Window& newWindow)
        : window(newWindow), core(newWindow), vertexBuffers(makeVertexBuffers(core.allocator())),
          colorPipeline(std::make_unique<ColorPipeline>(core.device(), core.colorFormat())) {}

    /// A swapchain rebuild may in theory change the color format (it does not on MoltenVK): then the pipeline must
    /// match the new one. Checked each frame, logged when it happens.
    void matchPipelineToSwapchain() {
        if (colorPipeline->colorFormat() == core.colorFormat()) {
            return;
        }
        logInfo("swapchain color format changed from {} to {}; rebuilding the color pipeline",
                static_cast<int>(colorPipeline->colorFormat()), static_cast<int>(core.colorFormat()));
        colorPipeline = std::make_unique<ColorPipeline>(core.device(), core.colorFormat());
    }

    const Window& window;
    RenderCore core;
    std::array<Buffer, kFramesInFlight> vertexBuffers; ///< one per frame slot, indexed by `core.frameIndex()`
    std::unique_ptr<ColorPipeline> colorPipeline;      ///< rebuilt only if the swapchain's color format changes
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
        m_impl->matchPipelineToSwapchain();
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
