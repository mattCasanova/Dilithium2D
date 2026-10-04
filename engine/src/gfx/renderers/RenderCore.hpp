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
    Ready,   ///< the frame is open: record into `commands()`, then call `endFrame()`
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
    /// On `Ready`, record draws into `commands()` and call `endFrame()`; on anything else, do not.
    [[nodiscard]] FrameBegin beginFrame(const Window& window, Color clearColor);

    /// Closes the frame `beginFrame` opened: ends rendering, submits, presents. A programmer error without one open.
    void endFrame();

    /// The open frame's command buffer and the size it draws at. Only between `beginFrame` (`Ready`) and `endFrame`.
    [[nodiscard]] VkCommandBuffer commands() const;
    [[nodiscard]] VkExtent2D extent() const;

    [[nodiscard]] uint32_t validationMessages() const { return m_instance.validationMessages(); }
    [[nodiscard]] uint32_t swapchainBuilds() const { return m_swapchainBuilds; }

private:
    void recreateSwapchain(PixelSize windowPixels);
    void beginCommands(const FrameSlot& frame, uint32_t imageIndex, Color clearColor) const;
    void endCommands(const FrameSlot& frame, uint32_t imageIndex) const;
    void submit(const FrameSlot& frame, uint32_t imageIndex) const;
    void present(uint32_t imageIndex);

    Instance m_instance;
    Surface m_surface;
    Device m_device;
    Allocator m_allocator;
    std::unique_ptr<Swapchain> m_swapchain; ///< null while the window has no area
    FramesInFlight m_frames;
    /// Acquire or present said the swapchain is out of date. A size change needs no flag: drawFrame compares sizes.
    bool m_swapchainStale = false;
    uint32_t m_swapchainBuilds = 0;
    bool m_frameOpen = false;       ///< between a Ready beginFrame and its endFrame
    uint32_t m_frameImageIndex = 0; ///< the swapchain image the open frame draws into
};

} // namespace dilithium
