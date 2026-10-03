#pragma once

#include <cstdint>

namespace dilithium {

/// The longest step one frame may take, as in LiquidMetal2D. After a stall (a breakpoint, a window drag) the game
/// moves one 1/15 s step instead of jumping.
inline constexpr float kMaxFrameSeconds = 1.0f / 15.0f;

/// Seconds between two readings of a monotonic nanosecond clock, clamped to `kMaxFrameSeconds`.
float frameSeconds(uint64_t previousNs, uint64_t nowNs);

} // namespace dilithium
