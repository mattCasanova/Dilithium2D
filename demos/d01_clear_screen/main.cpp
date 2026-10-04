#include <dilithium/App.hpp>
#include <dilithium/gfx/Color.hpp>
#include <dilithium/math/Math.hpp>

#include <memory>

namespace {

/// D1: a window that clears to a color. The hue goes all the way round every six seconds, which proves frames
/// advance. Until Phase 6 draws, the window stays empty and `clearColor` has no caller.
class ClearScreen final : public dilithium::App {
public:
    [[nodiscard]] dilithium::AppConfig config() const override { return {.title = "D1 Clear Screen"}; }

    void onUpdate(float dt) override {
        m_hueDegrees = dilithium::math::wrap(m_hueDegrees + (dt * kDegreesPerSecond), 0.0f, kDegreesPerTurn);
    }

    [[nodiscard]] dilithium::Color clearColor() const override {
        return dilithium::Color::fromHSV(m_hueDegrees, kSaturation, kValue);
    }

private:
    static constexpr float kDegreesPerTurn = dilithium::math::kDegreesPerTurn;
    static constexpr float kSecondsPerTurn = 6.0f;
    static constexpr float kDegreesPerSecond = kDegreesPerTurn / kSecondsPerTurn;
    static constexpr float kSaturation = 0.6f; ///< soft, not neon
    static constexpr float kValue = 0.9f;      ///< bright, not white

    float m_hueDegrees = 0.0f;
};

} // namespace

std::unique_ptr<dilithium::App> dilithium::createApp(int /*argc*/, char** /*argv*/) {
    return std::make_unique<ClearScreen>();
}
