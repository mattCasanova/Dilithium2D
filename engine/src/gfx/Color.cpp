#include <dilithium/gfx/Color.hpp>
#include <dilithium/math/Math.hpp>
#include <dilithium/utilities/Assert.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace dilithium {
namespace {

constexpr int kSectorCount = 6; ///< the hue wheel: red, yellow, green, cyan, blue, magenta
constexpr float kDegreesPerSector = math::kDegreesPerTurn / kSectorCount;

/// One sixth of the hue wheel. Within a sector one channel is at full chroma, one at none, and one ramps between.
enum class Sector { RedToYellow, YellowToGreen, GreenToCyan, CyanToBlue, BlueToMagenta, MagentaToRed };

} // namespace

Color Color::fromHSV(float hueDegrees, float saturation, float value, float alpha) {
    if (!std::isfinite(hueDegrees) || !std::isfinite(saturation) || !std::isfinite(value) || !std::isfinite(alpha)) {
        DILITHIUM_UNREACHABLE("Color::fromHSV given a non-finite input");
    }

    const float hue = math::wrap(hueDegrees, 0.0f, math::kDegreesPerTurn);
    const float s = std::clamp(saturation, 0.0f, 1.0f);
    const float v = std::clamp(value, 0.0f, 1.0f);
    const float a = std::clamp(alpha, 0.0f, 1.0f);

    const float chroma = v * s;
    const float position = hue / kDegreesPerSector; // [0, 6): sector number plus how far into it
    const float x = chroma * (1.0f - std::abs(std::fmod(position, 2.0f) - 1.0f));
    const float m = v - chroma;

    switch (static_cast<Sector>(static_cast<int>(position))) {
    case Sector::RedToYellow:
        return {chroma + m, x + m, m, a};
    case Sector::YellowToGreen:
        return {x + m, chroma + m, m, a};
    case Sector::GreenToCyan:
        return {m, chroma + m, x + m, a};
    case Sector::CyanToBlue:
        return {m, x + m, chroma + m, a};
    case Sector::BlueToMagenta:
        return {x + m, m, chroma + m, a};
    case Sector::MagentaToRed:
        return {chroma + m, m, x + m, a};
    }
    DILITHIUM_UNREACHABLE("hue sector outside the wheel after wrapping");
}

std::array<uint8_t, 4> Color::toRGBA8() const {
    const auto toByte = [](float component) {
        constexpr float kMax = 255.0f;
        return static_cast<uint8_t>(std::lround(std::clamp(component, 0.0f, 1.0f) * kMax));
    };
    return {toByte(r), toByte(g), toByte(b), toByte(a)};
}

} // namespace dilithium
