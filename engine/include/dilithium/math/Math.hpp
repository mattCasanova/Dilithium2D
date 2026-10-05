#pragma once

#include <dilithium/utilities/Assert.hpp>

#include <algorithm>
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

/// Where `value` sits between `a` and `b`: 0 at `a`, 1 at `b`, outside [0, 1] beyond them. The inverse of
/// `std::lerp`. `a == b` has no answer and is a programmer error.
inline float inverseLerp(float a, float b, float value) {
    if (a == b) {
        DILITHIUM_UNREACHABLE("math::inverseLerp needs two different ends");
    }
    return (value - a) / (b - a);
}

/// Maps `value` from one range to another, linearly, without clamping: 5 in [0, 10] is 50 in [0, 100].
inline float remap(float value, float fromLow, float fromHigh, float toLow, float toHigh) {
    return std::lerp(toLow, toHigh, inverseLerp(fromLow, fromHigh, value));
}

/// Interpolates between two angles in radians the short way round, so 350° to 10° turns through 0°, not 180°.
inline float lerpAngle(float a, float b, float t) {
    const float delta = wrap(b - a, -kPi, kPi);
    return a + (delta * t);
}

/// Ken Perlin's smoother step: like `std::smoothstep` (glm's `smoothstep`) but with zero second derivative at both
/// ends too, so motion starts and stops without a visible kink.
constexpr float smootherstep(float edge0, float edge1, float x) {
    const float t = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    constexpr float kA = 6.0f;
    constexpr float kB = 15.0f;
    constexpr float kC = 10.0f;
    return t * t * t * ((t * ((t * kA) - kB)) + kC);
}

} // namespace dilithium::math
