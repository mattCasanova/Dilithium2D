#include "engine/FrameClock.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <chrono>

using Catch::Matchers::WithinAbs;
using dilithium::FrameClock;
using dilithium::frameSeconds;
using dilithium::kMaxFrameSeconds;

TEST_CASE("frameSeconds turns an elapsed time into seconds", "[frameclock]") {
    CHECK_THAT(frameSeconds(std::chrono::nanoseconds{16'666'667}), WithinAbs(1.0 / 60.0, 1e-6));
    CHECK_THAT(frameSeconds(std::chrono::nanoseconds{0}), WithinAbs(0.0, 1e-9));
}

TEST_CASE("frameSeconds clamps a long stall to 1/15 s, like LiquidMetal2D", "[frameclock]") {
    CHECK(frameSeconds(std::chrono::seconds{1}) == kMaxFrameSeconds);
    CHECK(frameSeconds(std::chrono::hours{1}) == kMaxFrameSeconds);
    CHECK_THAT(frameSeconds(std::chrono::nanoseconds{66'666'667}), WithinAbs(1.0 / 15.0, 1e-6));
}

TEST_CASE("a FrameClock ticks forward from its construction, and a reset forgets the time away", "[frameclock]") {
    FrameClock clock;
    const float first = clock.tick();
    CHECK(first >= 0.0f);
    CHECK(first <= kMaxFrameSeconds);
    clock.reset();
    CHECK(clock.tick() < kMaxFrameSeconds);
}
