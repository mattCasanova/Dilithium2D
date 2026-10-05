#include "engine/FrameClock.hpp"

#include <dilithium/utilities/Assert.hpp>

#include <algorithm>
#include <chrono>

namespace dilithium {

float frameSeconds(std::chrono::nanoseconds elapsed) {
    DILITHIUM_ASSERT(elapsed.count() >= 0, "the frame clock went backwards");
    const double seconds = std::chrono::duration<double>(elapsed).count();
    return static_cast<float>(std::min(seconds, static_cast<double>(kMaxFrameSeconds)));
}

FrameClock::FrameClock() : last(std::chrono::steady_clock::now()) {}

float FrameClock::tick() {
    const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    const float seconds = frameSeconds(now - last);
    last = now;
    return seconds;
}

void FrameClock::reset() {
    last = std::chrono::steady_clock::now();
}

} // namespace dilithium
