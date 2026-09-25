#pragma once

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

    friend bool operator==(const Color&, const Color&) = default;
};

} // namespace dilithium
