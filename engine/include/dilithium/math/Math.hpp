#pragma once

#include <dilithium/core/Assert.hpp>

#include <cmath>
#include <numbers>

/// Constants and small helpers every area needs, the C++ form of LiquidMetal2D's `GameMath`. What the standard
/// library already does (`std::clamp`, `std::lerp`, `std::min`, `std::max`, `std::abs`) is not repeated here.
namespace dilithium::math {

inline constexpr float kPi = std::numbers::pi_v<float>;
inline constexpr float kHalfPi = kPi / 2.0f;
inline constexpr float kTwoPi = kPi * 2.0f;
inline constexpr float kDegreesPerTurn = 360.0f;
inline constexpr float kRadiansPerDegree = kTwoPi / kDegreesPerTurn;
inline constexpr float kDegreesPerRadian = kDegreesPerTurn / kTwoPi;

/// The tolerance `isNearlyEqual` uses when none is given, as in LiquidMetal2D and Mach 5.
inline constexpr float kEpsilon = 0.00001f;

constexpr float degreesToRadians(float degrees) {
    return degrees * kRadiansPerDegree;
}
constexpr float radiansToDegrees(float radians) {
    return radians * kDegreesPerRadian;
}

/// Whether `a` and `b` differ by no more than `epsilon`.
constexpr bool isNearlyEqual(float a, float b, float epsilon = kEpsilon) {
    const float difference = a - b;
    return difference <= epsilon && -difference <= epsilon;
}

/// Whether `value` lies in the closed range [low, high].
constexpr bool isInRange(float value, float low, float high) {
    return value >= low && value <= high;
}

/// Wraps `value` into [low, high): the overshoot carries over, so wrapping 370 into [0, 360) gives 10 and -10
/// gives 350. The result is never `high` itself, even when rounding would put it there. `low < high` is the
/// caller's job; anything else is a programmer error.
inline float wrap(float value, float low, float high) {
    if (!(low < high)) {
        DILITHIUM_UNREACHABLE("math::wrap needs low < high");
    }
    const float range = high - low;
    const float offset = std::fmod(value - low, range);
    const float wrapped = offset >= 0.0f ? low + offset : low + offset + range;
    // A tiny negative offset rounds up to exactly `range` after the add.
    return wrapped >= high ? low : wrapped;
}

} // namespace dilithium::math
