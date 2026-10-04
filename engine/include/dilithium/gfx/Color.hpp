#pragma once

#include <array>
#include <cstdint>

namespace dilithium {

/// RGBA, each component 0…1. The swapchain is `B8G8R8A8_UNORM`, like LiquidMetal2D's `.bgra8Unorm` layer, so the
/// same values look the same on screen in both engines.
struct Color {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;

    /// Hue in degrees, wrapped into 0…360 (so 360 is red again and -120 is 240). Saturation, value and alpha clamp
    /// to 0…1. A non-finite input is a bug and fails through `DILITHIUM_UNREACHABLE`.
    static Color fromHSV(float hueDegrees, float saturation, float value, float alpha = 1.0f);

    static constexpr uint8_t kOpaque = 0xFF; ///< the alpha byte of a solid color

    /// From bytes, 255 meaning 1.
    static constexpr Color fromRGBA8(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = kOpaque) {
        constexpr float kMax = 255.0f;
        return {static_cast<float>(red) / kMax, static_cast<float>(green) / kMax, static_cast<float>(blue) / kMax,
                static_cast<float>(alpha) / kMax};
    }

    /// From one hex number in the order a stylesheet writes it: `0xRRGGBBAA`, so `0xFF0000FF` is opaque red.
    static constexpr Color fromHex(uint32_t rgba) {
        constexpr uint32_t kRedShift = 24;
        constexpr uint32_t kGreenShift = 16;
        constexpr uint32_t kBlueShift = 8;
        constexpr uint32_t kByte = kOpaque;
        return fromRGBA8(static_cast<uint8_t>((rgba >> kRedShift) & kByte),
                         static_cast<uint8_t>((rgba >> kGreenShift) & kByte),
                         static_cast<uint8_t>((rgba >> kBlueShift) & kByte), static_cast<uint8_t>(rgba & kByte));
    }

    /// The four bytes, red first, as an `R8G8B8A8_UNORM` vertex attribute wants them in memory. Each component is
    /// clamped to 0…1 and rounded to the nearest byte.
    [[nodiscard]] std::array<uint8_t, 4> toRGBA8() const;

    friend bool operator==(const Color&, const Color&) = default;
};

/// The handful every game reaches for.
namespace colors {

inline constexpr Color kBlack{0.0f, 0.0f, 0.0f, 1.0f};
inline constexpr Color kWhite{1.0f, 1.0f, 1.0f, 1.0f};
inline constexpr Color kRed{1.0f, 0.0f, 0.0f, 1.0f};
inline constexpr Color kGreen{0.0f, 1.0f, 0.0f, 1.0f};
inline constexpr Color kBlue{0.0f, 0.0f, 1.0f, 1.0f};
inline constexpr Color kTransparent{0.0f, 0.0f, 0.0f, 0.0f};

} // namespace colors

} // namespace dilithium
