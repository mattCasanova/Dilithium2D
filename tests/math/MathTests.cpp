#include <dilithium/math/Math.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <numbers>

using Catch::Matchers::WithinAbs;
using namespace dilithium::math;

namespace {

/// Catch2's WithinAbs takes doubles; this keeps the widening explicit so -Wdouble-promotion stays quiet.
auto nearly(float expected) {
    constexpr double kTolerance = 1e-5;
    return WithinAbs(static_cast<double>(expected), kTolerance);
}

} // namespace

TEST_CASE("the constants come from <numbers>, not typed out", "[math]") {
    STATIC_CHECK(kPi == std::numbers::pi_v<float>);
    STATIC_CHECK(kHalfPi == kPi / 2.0f);
    STATIC_CHECK(kTwoPi == kPi * 2.0f);
    STATIC_CHECK(kDegreesPerTurn == 360.0f);
}

TEST_CASE("degrees and radians convert both ways", "[math]") {
    STATIC_CHECK(degreesToRadians(0.0f) == 0.0f);
    CHECK_THAT(degreesToRadians(180.0f), nearly(kPi));
    CHECK_THAT(degreesToRadians(90.0f), nearly(kHalfPi));
    CHECK_THAT(radiansToDegrees(kTwoPi), nearly(360.0f));
    CHECK_THAT(radiansToDegrees(degreesToRadians(123.0f)), nearly(123.0f));
}

TEST_CASE("isNearlyEqual uses the default epsilon or the one given", "[math]") {
    STATIC_CHECK(isNearlyEqual(1.0f, 1.0f));
    STATIC_CHECK(isNearlyEqual(1.0f, 1.0f + (kEpsilon / 2.0f)));
    STATIC_CHECK_FALSE(isNearlyEqual(1.0f, 1.001f));
    STATIC_CHECK(isNearlyEqual(1.0f, 1.001f, 0.01f));
    STATIC_CHECK(isNearlyEqual(-3.0f, -3.0f));
}

TEST_CASE("isInRange includes both ends", "[math]") {
    STATIC_CHECK(isInRange(0.0f, 0.0f, 1.0f));
    STATIC_CHECK(isInRange(1.0f, 0.0f, 1.0f));
    STATIC_CHECK(isInRange(0.5f, 0.0f, 1.0f));
    STATIC_CHECK_FALSE(isInRange(1.5f, 0.0f, 1.0f));
    STATIC_CHECK_FALSE(isInRange(-0.5f, 0.0f, 1.0f));
}

TEST_CASE("wrap keeps a value already in range", "[math]") {
    CHECK(wrap(10.0f, 0.0f, 360.0f) == 10.0f);
    CHECK(wrap(0.0f, 0.0f, 360.0f) == 0.0f);
    CHECK(wrap(-5.0f, -10.0f, 10.0f) == -5.0f);
}

TEST_CASE("wrap carries the overshoot over, both ways", "[math]") {
    CHECK_THAT(wrap(370.0f, 0.0f, 360.0f), nearly(10.0f));
    CHECK_THAT(wrap(-10.0f, 0.0f, 360.0f), nearly(350.0f));
    CHECK_THAT(wrap(725.0f, 0.0f, 360.0f), nearly(5.0f));
    CHECK_THAT(wrap(15.0f, -10.0f, 10.0f), nearly(-5.0f));
}

TEST_CASE("wrap never returns the high end", "[math]") {
    CHECK(wrap(360.0f, 0.0f, 360.0f) == 0.0f);
    CHECK(wrap(720.0f, 0.0f, 360.0f) == 0.0f);
    // -1e-7 wraps to 360 - 1e-7, which rounds to exactly 360 in float: that must come back as 0, not 360.
    CHECK(wrap(-1e-7f, 0.0f, 360.0f) == 0.0f);
}

TEST_CASE("wrap handles any magnitude without recursion", "[math]") {
    CHECK(isInRange(wrap(1e30f, 0.0f, 360.0f), 0.0f, 360.0f));
    CHECK(isInRange(wrap(-1e30f, 0.0f, 360.0f), 0.0f, 360.0f));
}

#ifndef DILITHIUM_DEBUG
#include <stdexcept>

TEST_CASE("wrap rejects an empty or backwards range in release", "[math][assert]") {
    CHECK_THROWS_AS(wrap(1.0f, 5.0f, 5.0f), std::logic_error);
    CHECK_THROWS_AS(wrap(1.0f, 10.0f, 0.0f), std::logic_error);
}
#endif
