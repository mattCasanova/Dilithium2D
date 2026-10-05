#pragma once

#include <chrono>

namespace dilithium {

/// The longest step one frame may take, as in LiquidMetal2D. After a stall (a breakpoint, a window drag) the game
/// moves one 1/15 s step instead of jumping.
inline constexpr float kMaxFrameSeconds = 1.0f / 15.0f;

/// Seconds for an elapsed time, clamped to `kMaxFrameSeconds`. Pure, so it is tested as a table.
[[nodiscard]] float frameSeconds(std::chrono::nanoseconds elapsed);

/// The frame clock. Each `tick` returns the seconds since the one before, clamped; `reset` forgets the time away,
/// so the first frame after a freeze has a normal `dt`. Reads the standard monotonic clock.
class FrameClock {
public:
    FrameClock();

    [[nodiscard]] float tick();
    void reset();

private:
    std::chrono::steady_clock::time_point m_last;
};

} // namespace dilithium
