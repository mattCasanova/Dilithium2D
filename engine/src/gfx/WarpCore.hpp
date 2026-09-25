#pragma once

#include "gfx/Allocator.hpp"
#include "gfx/Device.hpp"
#include "gfx/HeisenbergCompensator.hpp"
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
/// it. Device lost anywhere is reported as a warp core breach (see `KHAAAN`).
class WarpCore {
public:
    explicit WarpCore(const Window& window);
    ~WarpCore();

    WarpCore(const WarpCore&) = delete;
    WarpCore& operator=(const WarpCore&) = delete;
    WarpCore(WarpCore&&) = delete;
    WarpCore& operator=(WarpCore&&) = delete;

    /// Clears the window to `color` and presents it. D1 only: D2 splits this into begin and end with drawing between.
    FrameOutcome drawFrame(const Window& window, Color color);

    /// The window's pixel size changed: rebuild the swapchain before the next frame.
    void notifyResized() { m_swapchainStale = true; }

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
    HeisenbergCompensator m_frames;
    bool m_swapchainStale = false;
};

} // namespace dilithium
