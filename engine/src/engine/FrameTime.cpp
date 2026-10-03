#include "engine/FrameTime.hpp"

#include <dilithium/core/Assert.hpp>

#include <algorithm>

namespace dilithium {

float frameSeconds(uint64_t previousNs, uint64_t nowNs) {
    DILITHIUM_ASSERT(nowNs >= previousNs, "the frame clock went backwards");
    const double seconds = static_cast<double>(nowNs - previousNs) / 1e9;
    return static_cast<float>(std::min(seconds, static_cast<double>(kMaxFrameSeconds)));
}

} // namespace dilithium
