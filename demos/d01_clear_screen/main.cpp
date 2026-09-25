#include <dilithium/App.hpp>
#include <dilithium/gfx/Color.hpp>

#include <cmath>
#include <memory>

namespace {

/// D1: a window that clears to a color. The hue goes all the way round every six seconds, which proves frames
/// advance. Until Phase 6 draws, the window stays empty and `clearColor` has no caller.
class ClearScreen final : public dilithium::App {
public:
    dilithium::AppConfig config() const override { return {.title = "D1 Clear Screen", .width = 1280, .height = 720}; }

    void onUpdate(float dt) override { m_hueDegrees = std::fmod(m_hueDegrees + dt * kDegreesPerSecond, 360.0f); }

    dilithium::Color clearColor() const override { return dilithium::Color::fromHSV(m_hueDegrees, 0.6f, 0.9f); }

private:
    static constexpr float kDegreesPerSecond = 360.0f / 6.0f;
    float m_hueDegrees = 0.0f;
};

} // namespace

std::unique_ptr<dilithium::App> dilithium::createApp(int /*argc*/, char** /*argv*/) {
    return std::make_unique<ClearScreen>();
}
