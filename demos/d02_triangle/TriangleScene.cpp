#include "TriangleScene.hpp"

#include <dilithium/gfx/Color.hpp>
#include <dilithium/gfx/Renderer.hpp>
#include <dilithium/math/Math.hpp>
#include <dilithium/math/Types.hpp>
#include <dilithium/scenes/SceneServices.hpp>

#include <cmath>

namespace demo {
namespace {

constexpr float kSecondsPerTurn = 10.0f;
constexpr float kRadiansPerSecond = dilithium::math::kTwoPi / kSecondsPerTurn;
constexpr float kRadius = 0.6f; // in clip space: 0.6 of the way from the center to the edge
constexpr int kCorners = 3;
constexpr float kCornerSpacing = dilithium::math::kTwoPi / kCorners;
constexpr dilithium::Color kBackground{0.1f, 0.1f, 0.12f, 1.0f};

/// The corners sit on a circle, a third of a turn apart, the first straight up before any turning.
dilithium::Vec2 corner(float angle, int index) {
    const float at = angle + dilithium::math::kHalfPi + (static_cast<float>(index) * kCornerSpacing);
    return {kRadius * std::cos(at), kRadius * std::sin(at)};
}

} // namespace

TriangleScene::TriangleScene(dilithium::SceneServices& services) : renderer(services.renderer) {}

void TriangleScene::update(float dt) {
    angle = dilithium::math::wrap(angle + (dt * kRadiansPerSecond), 0.0f, dilithium::math::kTwoPi);
}

void TriangleScene::draw() {
    renderer.setClearColor(kBackground);
    renderer.drawTriangle(corner(angle, 0), corner(angle, 1), corner(angle, 2), dilithium::colors::kRed,
                          dilithium::colors::kGreen, dilithium::colors::kBlue);
}

} // namespace demo
