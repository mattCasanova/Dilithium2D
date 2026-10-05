#pragma once

#include "gfx/swapchain/FramesInFlight.hpp"
#include "gfx/swapchain/Swapchain.hpp"
#include "gfx/vulkan/Allocator.hpp"
#include "gfx/vulkan/Device.hpp"
#include "gfx/vulkan/Instance.hpp"
#include "gfx/vulkan/Surface.hpp"

#include <dilithium/gfx/Color.hpp>
#include <dilithium/gfx/Renderer.hpp>

#include <cstdint>
#include <memory>

namespace dilithium {

class Window;
struct PixelSize;

/// What `RenderCore::beginFrame` found.
enum class FrameBegin {
    Ready,   ///< the frame is open: record into `getCommands()`, then call `endFrame()`
    Skipped, ///< the swapchain went out of date; it is rebuilt at the start of the next frame, so call again now
    Idle,    ///< the window is minimized or has no area; nothing paced the frame, so the caller should sleep
};

/// Owns the GPU side of the engine (LiquidMetal2D's RenderCore): instance, surface, device, allocator, swapchain and
/// frame sync. Members are declared in creation order, so they are destroyed in the reverse order with no code for
/// it. A lost device anywhere fails with a "device lost" report (see `VK_CHECK`).
class RenderCore {
public:
    explicit RenderCore(const Window& window);
    ~RenderCore();

    RenderCore(const RenderCore&) = delete;
    RenderCore& operator=(const RenderCore&) = delete;
    RenderCore(RenderCore&&) = delete;
    RenderCore& operator=(RenderCore&&) = delete;

    /// Opens a frame: waits for its slot, acquires an image, and begins rendering into it, cleared to `clearColor`.
    /// On `Ready`, record draws into `getCommands()` and call `endFrame()`; on anything else, do not.
    [[nodiscard]] FrameBegin beginFrame(const Window& window, Color clearColor);

    /// Closes the frame `beginFrame` opened: ends rendering, submits, presents. A programmer error without one open.
    void endFrame();

    /// Waits until the GPU has finished everything submitted. Anything a submitted frame may still be using (a
    /// pipeline, a vertex buffer) must wait for this before it is destroyed. Logs a failure rather than throwing,
    /// so it is safe in a destructor.
    void waitIdle() const;

    /// The open frame's command buffer and the size it draws at. Only between `beginFrame` (`Ready`) and `endFrame`.
    [[nodiscard]] VkCommandBuffer getCommands() const;
    [[nodiscard]] VkExtent2D getExtent() const;

    /// Which frame slot the next (or open) frame uses: 0 or 1. A renderer keeps per-slot buffers under this index,
    /// so the CPU never writes a buffer the GPU may still be reading; the slot's fence, waited on in `beginFrame`,
    /// guarantees it.
    [[nodiscard]] uint32_t getFrameIndex() const { return frames.getIndex(); }
    [[nodiscard]] VmaAllocator getAllocator() const { return allocator.handle(); }
    [[nodiscard]] VkDevice getDevice() const { return device.handle(); }

    /// The swapchain's color format: what a pipeline that draws into it must be built for. It has not changed on
    /// MoltenVK; the renderer compares it each frame rather than assuming.
    [[nodiscard]] VkFormat getColorFormat() const;

    [[nodiscard]] uint32_t getValidationMessages() const { return instance.getValidationMessages(); }
    [[nodiscard]] uint32_t getSwapchainBuilds() const { return swapchainBuilds; }

private:
    void recreateSwapchain(PixelSize windowPixels);
    void beginCommands(const FrameSlot& frame, uint32_t imageIndex, Color clearColor) const;
    void endCommands(const FrameSlot& frame, uint32_t imageIndex) const;
    void submit(const FrameSlot& frame, uint32_t imageIndex) const;
    void present(uint32_t imageIndex);

    Instance instance;
    Surface surface;
    Device device;
    Allocator allocator;
    std::unique_ptr<Swapchain> swapchain; ///< null while the window has no area
    FramesInFlight frames;
    /// Acquire or present said the swapchain is out of date. A size change needs no flag: drawFrame compares sizes.
    bool swapchainStale = false;
    uint32_t swapchainBuilds = 0;
    bool frameOpen = false;       ///< between a Ready beginFrame and its endFrame
    uint32_t frameImageIndex = 0; ///< the swapchain image the open frame draws into
};

} // namespace dilithium
