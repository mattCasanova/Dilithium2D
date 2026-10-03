#pragma once

#include <cstdint>

namespace dilithium {

enum class TortureAction { None, Resize, Minimize, Restore };

/// What `--torture` does to the window on one frame. `width` and `height` (in points) are set for `Resize` only.
struct TortureStep {
    TortureAction action = TortureAction::None;
    int width = 0;
    int height = 0;
};

inline constexpr uint64_t kTortureResizeEvery = 20;
inline constexpr uint64_t kTortureMinimizeEvery = 97;
inline constexpr uint64_t kTortureRestoreAfter = 10;

/// The torture schedule, by frame (loop tick, counting from 1): every 20 frames resize to the next size in a fixed
/// list, from large to tiny and wide to tall; every 97 frames minimize, and restore 10 frames later. Minimize and
/// restore win over a resize due on the same frame.
TortureStep tortureStep(uint64_t frame);

} // namespace dilithium
