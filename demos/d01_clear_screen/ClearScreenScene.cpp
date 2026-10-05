#include "ClearScreenScene.hpp"

#include <dilithium/gfx/Color.hpp>
#include <dilithium/gfx/Renderer.hpp>
#include <dilithium/math/Math.hpp>
#include <dilithium/platform/AppState.hpp>
#include <dilithium/scenes/SceneServices.hpp>
#include <dilithium/utilities/Log.hpp>

namespace demo {
namespace {

constexpr float kSecondsPerTurn = 6.0f;
constexpr float kDegreesPerSecond = dilithium::math::kDegreesPerTurn / kSecondsPerTurn;
constexpr float kSaturation = 0.6f; // soft, not neon
constexpr float kValue = 0.9f;      // bright, not white

} // namespace

ClearScreenScene::ClearScreenScene(dilithium::SceneServices& services) : renderer(services.renderer) {}

void ClearScreenScene::update(float dt) {
    hueDegrees = dilithium::math::wrap(hueDegrees + (dt * kDegreesPerSecond), 0.0f, dilithium::math::kDegreesPerTurn);
}

void ClearScreenScene::draw() {
    renderer.setClearColor(dilithium::Color::fromHSV(hueDegrees, kSaturation, kValue));
}

void ClearScreenScene::appStateChanged(dilithium::AppState state) {
    dilithium::logInfo("app state: {}", dilithium::appStateName(state));
}

} // namespace demo
