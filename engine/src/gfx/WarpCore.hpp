#pragma once

#include "gfx/Allocator.hpp"
#include "gfx/Device.hpp"
#include "gfx/Instance.hpp"
#include "gfx/Surface.hpp"

namespace dilithium {

class Window;

/// Owns the GPU side of the engine (LiquidMetal2D's RenderCore): instance, surface, device and allocator, and from
/// Phase 6 the swapchain and the frame sync. Members are declared in creation order, so they are destroyed in the
/// reverse order with no code for it. Device lost anywhere is reported as a warp core breach (see `KHAAAN`).
class WarpCore {
public:
    explicit WarpCore(const Window& window);
    ~WarpCore();

    WarpCore(const WarpCore&) = delete;
    WarpCore& operator=(const WarpCore&) = delete;
    WarpCore(WarpCore&&) = delete;
    WarpCore& operator=(WarpCore&&) = delete;

private:
    Instance m_instance;
    Surface m_surface;
    Device m_device;
    Allocator m_allocator;
};

} // namespace dilithium
