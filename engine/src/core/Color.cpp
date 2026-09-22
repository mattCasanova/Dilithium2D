#include <dilithium/Assert.hpp>
#include <dilithium/Color.hpp>

#include <algorithm>
#include <cmath>

namespace dilithium {
namespace {

/// Wraps any finite angle into [0, 360).
float wrapDegrees(float degrees) {
    float wrapped = std::fmod(degrees, 360.0f);
    if (wrapped < 0.0f) {
        wrapped += 360.0f;
    }
    // A tiny negative input rounds up to exactly 360 after the add.
    return wrapped >= 360.0f ? 0.0f : wrapped;
}

} // namespace

Color Color::fromHSV(float hueDegrees, float saturation, float value, float alpha) {
    if (!std::isfinite(hueDegrees) || !std::isfinite(saturation) || !std::isfinite(value) || !std::isfinite(alpha)) {
        ILLOGICAL("Color::fromHSV given a non-finite input");
    }

    const float hue = wrapDegrees(hueDegrees);
    const float s = std::clamp(saturation, 0.0f, 1.0f);
    const float v = std::clamp(value, 0.0f, 1.0f);
    const float a = std::clamp(alpha, 0.0f, 1.0f);

    const float chroma = v * s;
    const float sector = hue / 60.0f; // [0, 6)
    const float x = chroma * (1.0f - std::abs(std::fmod(sector, 2.0f) - 1.0f));
    const float m = v - chroma;

    switch (static_cast<int>(sector)) {
    case 0:
        return {chroma + m, x + m, m, a};
    case 1:
        return {x + m, chroma + m, m, a};
    case 2:
        return {m, chroma + m, x + m, a};
    case 3:
        return {m, x + m, chroma + m, a};
    case 4:
        return {x + m, m, chroma + m, a};
    case 5:
        return {chroma + m, m, x + m, a};
    }
    ILLOGICAL("hue sector outside 0...5 after wrapping");
}

} // namespace dilithium
