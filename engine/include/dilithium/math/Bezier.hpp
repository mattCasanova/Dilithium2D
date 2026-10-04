#pragma once

#include <dilithium/math/Types.hpp>

namespace dilithium::math {

// `inline`, not `constexpr`: glm's aligned vectors cannot take part in a constant expression.

/// The point at `t` in [0, 1] on the curve from `p0` to `p2`, pulled toward `p1`.
inline Vec2 quadraticBezier(Vec2 p0, Vec2 p1, Vec2 p2, float t) {
    const float u = 1.0f - t;
    return (u * u * p0) + (2.0f * u * t * p1) + (t * t * p2);
}

/// The point at `t` in [0, 1] on the curve from `p0` to `p3`, pulled toward `p1` then `p2`.
inline Vec2 cubicBezier(Vec2 p0, Vec2 p1, Vec2 p2, Vec2 p3, float t) {
    const float u = 1.0f - t;
    constexpr float kThree = 3.0f;
    return (u * u * u * p0) + (kThree * u * u * t * p1) + (kThree * u * t * t * p2) + (t * t * t * p3);
}

} // namespace dilithium::math
