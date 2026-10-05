#include "StressScene.hpp"

#include "StressSchedule.hpp"

#include <dilithium/core/Assert.hpp>
#include <dilithium/engine/Application.hpp>
#include <dilithium/gfx/Color.hpp>
#include <dilithium/gfx/Renderer.hpp>
#include <dilithium/math/Math.hpp>
#include <dilithium/math/Types.hpp>
#include <dilithium/platform/Window.hpp>
#include <dilithium/scenes/SceneServices.hpp>

#include <cmath>

namespace stress {
namespace {

constexpr float kSecondsPerHueTurn = 6.0f;
constexpr float kHueDegreesPerSecond = dilithium::math::kDegreesPerTurn / kSecondsPerHueTurn;
constexpr float kSaturation = 0.6f;
constexpr float kValue = 0.9f;

constexpr float kSecondsPerTriangleTurn = 10.0f;
constexpr float kRadiansPerSecond = dilithium::math::kTwoPi / kSecondsPerTriangleTurn;
constexpr float kRadius = 0.6f;
constexpr int kCorners = 3;
constexpr float kCornerSpacing = dilithium::math::kTwoPi / kCorners;

void apply(dilithium::Window& window, Step step) {
    switch (step.action) {
    case Action::None:
        return;
    case Action::Resize:
        window.resize(step.width, step.height);
        return;
    case Action::Minimize:
        window.minimize();
        return;
    }
    DILITHIUM_UNREACHABLE("unknown stress Action");
}

/// The corners sit on a circle, a third of a turn apart, the first straight up before any turning.
dilithium::Vec2 corner(float angle, int index) {
    const float at = angle + dilithium::math::kHalfPi + (static_cast<float>(index) * kCornerSpacing);
    return {kRadius * std::cos(at), kRadius * std::sin(at)};
}

} // namespace

StressScene::StressScene(dilithium::SceneServices& services, dilithium::Window& stressed)
    : renderer(services.renderer), window(stressed) {}

void StressScene::update(float dt) {
    ++frame;
    apply(window, stepFor(frame));
    hueDegrees =
        dilithium::math::wrap(hueDegrees + (dt * kHueDegreesPerSecond), 0.0f, dilithium::math::kDegreesPerTurn);
    angle = dilithium::math::wrap(angle + (dt * kRadiansPerSecond), 0.0f, dilithium::math::kTwoPi);
}

void StressScene::draw() {
    renderer.setClearColor(dilithium::Color::fromHSV(hueDegrees, kSaturation, kValue));
    renderer.drawTriangle(corner(angle, 0), corner(angle, 1), corner(angle, 2), dilithium::colors::kRed,
                          dilithium::colors::kGreen, dilithium::colors::kBlue);
}

void StressScene::appStateChanged(dilithium::AppState state) {
    if (state == dilithium::AppState::Background) {
        window.restore();
    }
}

} // namespace stress
