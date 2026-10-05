#include "gfx/buffers/Buffer.hpp"
#include "gfx/renderers/ColorVertex.hpp"
#include "gfx/renderers/RenderCore.hpp"
#include "gfx/shaders/ColorPipeline.hpp"
#include "gfx/swapchain/FramesInFlight.hpp"

#include <dilithium/gfx/Color.hpp>
#include <dilithium/gfx/Renderer.hpp>
#include <dilithium/gfx/renderers/DefaultRenderer.hpp>
#include <dilithium/math/Types.hpp>
#include <dilithium/utilities/Assert.hpp>
#include <dilithium/utilities/Log.hpp>

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <array>
#include <cstdint>
#include <memory>
#include <span>
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
        : window(newWindow), core(newWindow), vertexBuffers(makeVertexBuffers(core.getAllocator())),
          colorPipeline(std::make_unique<ColorPipeline>(core.getDevice(), core.getColorFormat())) {}

    /// Draws this frame's recorded triangles: the vertices go into this slot's buffer (the GPU finished with it,
    /// the slot's fence said so in beginFrame), then one draw.
    void drawTriangles() {
        if (triangles.empty()) {
            return;
        }
        if (triangles.size() > kMaxColorVertices) {
            DILITHIUM_UNREACHABLE("more triangle vertices in one frame than the vertex buffer holds");
        }
        Buffer& buffer = vertexBuffers.at(core.getFrameIndex());
        buffer.write(std::as_bytes(std::span(triangles)));

        const VkCommandBuffer commands = core.getCommands();
        const VkExtent2D extent = core.getExtent();
        // A negative height flips Vulkan's +Y-down clip space, so +Y is up as in LiquidMetal2D (D2's D-3). Core
        // since Vulkan 1.1; the origin moves to the bottom-left to compensate.
        const VkViewport viewport{
            .x = 0.0f,
            .y = static_cast<float>(extent.height),
            .width = static_cast<float>(extent.width),
            .height = -static_cast<float>(extent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };
        vkCmdSetViewport(commands, 0, 1, &viewport);
        const VkRect2D scissor{.offset = {0, 0}, .extent = extent};
        vkCmdSetScissor(commands, 0, 1, &scissor);

        vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, colorPipeline->handle());
        const VkBuffer vertexBuffer = buffer.handle();
        const VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(commands, 0, 1, &vertexBuffer, &offset);
        vkCmdDraw(commands, static_cast<uint32_t>(triangles.size()), 1, 0, 0);
    }

    /// A swapchain rebuild may in theory change the color format (it does not on MoltenVK): then the pipeline must
    /// match the new one. Checked each frame, logged when it happens.
    void matchPipelineToSwapchain() {
        if (colorPipeline->getColorFormat() == core.getColorFormat()) {
            return;
        }
        logInfo("swapchain color format changed from {} to {}; rebuilding the color pipeline",
                static_cast<int>(colorPipeline->getColorFormat()), static_cast<int>(core.getColorFormat()));
        colorPipeline = std::make_unique<ColorPipeline>(core.getDevice(), core.getColorFormat());
    }

    const Window& window;
    RenderCore core;
    std::array<Buffer, kFramesInFlight> vertexBuffers; ///< one per frame slot, indexed by `core.getFrameIndex()`
    std::unique_ptr<ColorPipeline> colorPipeline;      ///< rebuilt only if the swapchain's color format changes
    Color clearColor;                   ///< what the current frame starts from; black until a scene says otherwise
    std::vector<ColorVertex> triangles; ///< what the scene asked to draw this frame, three vertices each
};

DefaultRenderer::DefaultRenderer(const Window& window) : impl(std::make_unique<Impl>(window)) {}

// NOLINTNEXTLINE(bugprone-exception-escape): it logs; std::format's bad_alloc at shutdown may end the program, rightly
DefaultRenderer::~DefaultRenderer() {
    // The pipeline and the vertex buffers are destroyed before RenderCore is (reverse declaration order), and the
    // last frame submitted may still be using them: wait for the GPU first. Validation caught this one.
    impl->core.waitIdle();
    logInfo("renderer: {} swapchain builds", impl->core.getSwapchainBuilds());
}

void DefaultRenderer::setClearColor(Color color) {
    impl->clearColor = color;
}

void DefaultRenderer::drawTriangle(Vec2 a, Vec2 b, Vec2 c, Color colorA, Color colorB, Color colorC) {
    impl->triangles.push_back({a, colorA});
    impl->triangles.push_back({b, colorB});
    impl->triangles.push_back({c, colorC});
}

FrameOutcome DefaultRenderer::drawFrame() {
    // Whatever happens, the recorded list belongs to this frame alone.
    std::vector<ColorVertex>& triangles = impl->triangles;
    const auto dropRecording = [&triangles] { triangles.clear(); };

    switch (impl->core.beginFrame(impl->window, impl->clearColor)) {
    case FrameBegin::Ready:
        impl->matchPipelineToSwapchain();
        impl->drawTriangles();
        impl->core.endFrame();
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

uint32_t DefaultRenderer::getProblemsReported() const {
    return impl->core.getValidationMessages();
}

uint32_t DefaultRenderer::getSwapchainBuilds() const {
    return impl->core.getSwapchainBuilds();
}

} // namespace dilithium
