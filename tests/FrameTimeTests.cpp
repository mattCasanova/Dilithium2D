#include "core/FrameTime.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using Catch::Matchers::WithinAbs;
using dilithium::frameSeconds;
using dilithium::kMaxFrameSeconds;

TEST_CASE("frameSeconds turns nanoseconds into seconds", "[frametime]") {
    CHECK_THAT(frameSeconds(1'000'000'000, 1'016'666'667), WithinAbs(1.0 / 60.0, 1e-6));
    CHECK_THAT(frameSeconds(5'000, 5'000), WithinAbs(0.0, 1e-9));
}

TEST_CASE("frameSeconds clamps a long stall to 1/15 s, like LiquidMetal2D", "[frametime]") {
    CHECK(frameSeconds(0, 1'000'000'000) == kMaxFrameSeconds);
    CHECK(frameSeconds(0, 3'600'000'000'000) == kMaxFrameSeconds);
    CHECK_THAT(frameSeconds(0, 66'666'667), WithinAbs(1.0 / 15.0, 1e-6));
}
