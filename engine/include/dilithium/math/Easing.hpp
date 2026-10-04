#pragma once

#include <dilithium/math/Math.hpp>

#include <cmath>

/// Easing curves: each maps `t` in [0, 1] to a progress in [0, 1], 0 at 0 and 1 at 1, with a shape between.
/// LiquidMetal2D's `Easing`, the same 24 (Robert Penner's set). `in` starts slow, `out` ends slow, `inOut` both.
/// Elastic and back leave [0, 1] on purpose: they overshoot.
namespace dilithium::easing {

namespace detail {

inline constexpr float kHalf = 0.5f;
inline constexpr float kExpoSteepness = 10.0f;       ///< 2^(10 (t - 1)): a thousandfold rise over the range
inline constexpr float kElasticOscillations = 13.0f; ///< sin(13 pi / 2 t): three and a quarter swings
inline constexpr float kBackOvershoot = 1.70158f;    ///< Penner's constant: about 10% past the end
inline constexpr float kBackInOutScale = 1.525f;     ///< the overshoot for the halved in-out form
inline constexpr float kBounceSpan = 2.75f;          ///< the four bounces fit in 2.75 units of t
inline constexpr float kBounceStiffness = 7.5625f;   ///< 2.75 squared: each bounce is a parabola of this width

} // namespace detail

// --- quadratic

constexpr float inQuad(float t) {
    return t * t;
}
constexpr float outQuad(float t) {
    return t * (2.0f - t);
}
constexpr float inOutQuad(float t) {
    return t < detail::kHalf ? 2.0f * t * t : -1.0f + ((4.0f - (2.0f * t)) * t);
}

// --- cubic

constexpr float inCubic(float t) {
    return t * t * t;
}
constexpr float outCubic(float t) {
    const float u = t - 1.0f;
    return (u * u * u) + 1.0f;
}
constexpr float inOutCubic(float t) {
    if (t < detail::kHalf) {
        return 4.0f * t * t * t;
    }
    const float u = (2.0f * t) - 2.0f;
    return ((u * u * u) + 2.0f) / 2.0f;
}

// --- quartic

constexpr float inQuart(float t) {
    return t * t * t * t;
}
constexpr float outQuart(float t) {
    const float u = t - 1.0f;
    return 1.0f - (u * u * u * u);
}
constexpr float inOutQuart(float t) {
    constexpr float kEight = 8.0f;
    if (t < detail::kHalf) {
        return kEight * t * t * t * t;
    }
    const float u = t - 1.0f;
    return 1.0f - (kEight * u * u * u * u);
}

// --- sine

inline float inSine(float t) {
    return 1.0f - std::cos(t * math::kHalfPi);
}
inline float outSine(float t) {
    return std::sin(t * math::kHalfPi);
}
inline float inOutSine(float t) {
    return (1.0f - std::cos(math::kPi * t)) / 2.0f;
}

// --- exponential

inline float inExpo(float t) {
    return t == 0.0f ? 0.0f : std::exp2(detail::kExpoSteepness * (t - 1.0f));
}
inline float outExpo(float t) {
    return t == 1.0f ? 1.0f : 1.0f - std::exp2(-detail::kExpoSteepness * t);
}
inline float inOutExpo(float t) {
    if (t == 0.0f) {
        return 0.0f;
    }
    if (t == 1.0f) {
        return 1.0f;
    }
    const float doubled = 2.0f * detail::kExpoSteepness;
    if (t < detail::kHalf) {
        return std::exp2((doubled * t) - detail::kExpoSteepness) / 2.0f;
    }
    return (2.0f - std::exp2((-doubled * t) + detail::kExpoSteepness)) / 2.0f;
}

// --- elastic (overshoots)

inline float inElastic(float t) {
    return std::sin(detail::kElasticOscillations * math::kHalfPi * t) * std::exp2(detail::kExpoSteepness * (t - 1.0f));
}
inline float outElastic(float t) {
    return (std::sin(-detail::kElasticOscillations * math::kHalfPi * (t + 1.0f)) *
            std::exp2(-detail::kExpoSteepness * t)) +
           1.0f;
}
inline float inOutElastic(float t) {
    const float swing = detail::kElasticOscillations * math::kPi * t;
    const float stretch = detail::kExpoSteepness * ((2.0f * t) - 1.0f);
    if (t < detail::kHalf) {
        return detail::kHalf * std::sin(swing) * std::exp2(stretch);
    }
    return detail::kHalf * ((std::sin(-swing) * std::exp2(-stretch)) + 2.0f);
}

// --- bounce

constexpr float outBounce(float t) {
    using detail::kBounceSpan;
    using detail::kBounceStiffness;
    // Four parabolas, each a smaller bounce: the thresholds are where one ends and the next begins.
    constexpr float kFirstEnd = 1.0f / kBounceSpan;
    constexpr float kSecondEnd = 2.0f / kBounceSpan;
    constexpr float kThirdEnd = 2.5f / kBounceSpan;
    if (t < kFirstEnd) {
        return kBounceStiffness * t * t;
    }
    if (t < kSecondEnd) {
        const float u = t - (1.5f / kBounceSpan);
        return (kBounceStiffness * u * u) + 0.75f;
    }
    if (t < kThirdEnd) {
        const float u = t - (2.25f / kBounceSpan);
        return (kBounceStiffness * u * u) + 0.9375f;
    }
    const float u = t - (2.625f / kBounceSpan);
    return (kBounceStiffness * u * u) + 0.984375f;
}
constexpr float inBounce(float t) {
    return 1.0f - outBounce(1.0f - t);
}
constexpr float inOutBounce(float t) {
    if (t < detail::kHalf) {
        return (1.0f - outBounce(1.0f - (2.0f * t))) / 2.0f;
    }
    return (1.0f + outBounce((2.0f * t) - 1.0f)) / 2.0f;
}

// --- back (overshoots)

constexpr float inBack(float t) {
    constexpr float kS = detail::kBackOvershoot;
    return t * t * (((kS + 1.0f) * t) - kS);
}
constexpr float outBack(float t) {
    constexpr float kS = detail::kBackOvershoot;
    const float u = t - 1.0f;
    return (u * u * (((kS + 1.0f) * u) + kS)) + 1.0f;
}
constexpr float inOutBack(float t) {
    constexpr float kS = detail::kBackOvershoot * detail::kBackInOutScale;
    if (t < detail::kHalf) {
        return (4.0f * t * t * (((kS + 1.0f) * 2.0f * t) - kS)) / 2.0f;
    }
    const float u = (2.0f * t) - 2.0f;
    return ((u * u * (((kS + 1.0f) * u) + kS)) + 2.0f) / 2.0f;
}

} // namespace dilithium::easing
