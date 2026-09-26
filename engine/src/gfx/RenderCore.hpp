#pragma once

#include "gfx/Allocator.hpp"
#include "gfx/Device.hpp"
#include "gfx/FramesInFlight.hpp"
#include "gfx/Instance.hpp"
#include "gfx/Surface.hpp"
#include "gfx/Swapchain.hpp"

#include <dilithium/gfx/Color.hpp>

#include <cstdint>
#include <memory>

namespace dilithium {

class Window;
struct PixelSize;

enum class FrameOutcome {
    Presented, ///< drawn and handed to the screen; FIFO present paced the frame
    Skipped,   ///< the swapchain went out of date; it is rebuilt at the start of the next frame, so call again now
    Idle,      ///< the window is minimized or has no area; nothing paced the frame, so the caller should sleep
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

    /// Clears the window to `color` and presents it. D1 only: D2 splits this into begin and end with drawing between.
    FrameOutcome drawFrame(const Window& window, Color color);

    /// The window's pixel size changed: rebuild the swapchain before the next frame.
    void notifyResized() { m_swapchainStale = true; }

    uint32_t validationMessages() const { return m_instance.validationMessages(); }
    uint32_t swapchainBuilds() const { return m_swapchainBuilds; }

private:
    void recreateSwapchain(PixelSize windowPixels);
    void recordClear(const FrameSlot& frame, uint32_t imageIndex, Color color) const;
    void submit(const FrameSlot& frame, uint32_t imageIndex) const;
    void present(uint32_t imageIndex);

    Instance m_instance;
    Surface m_surface;
    Device m_device;
    Allocator m_allocator;
    std::unique_ptr<Swapchain> m_swapchain; ///< null while the window has no area
    FramesInFlight m_frames;
    bool m_swapchainStale = false;
    uint32_t m_swapchainBuilds = 0;
};

} // namespace dilithium
