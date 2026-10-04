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

    [[nodiscard]] uint32_t validationMessages() const { return m_instance.validationMessages(); }
    [[nodiscard]] uint32_t swapchainBuilds() const { return m_swapchainBuilds; }

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
    /// Acquire or present said the swapchain is out of date. A size change needs no flag: drawFrame compares sizes.
    bool m_swapchainStale = false;
    uint32_t m_swapchainBuilds = 0;
};

} // namespace dilithium
