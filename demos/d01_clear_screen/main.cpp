#include <dilithium/core/Assert.hpp>
#include <dilithium/core/Log.hpp>
#include <dilithium/engine/Application.hpp>
#include <dilithium/engine/CommandLine.hpp>
#include <dilithium/engine/Engine.hpp>
#include <dilithium/gfx/Color.hpp>
#include <dilithium/gfx/Renderer.hpp>
#include <dilithium/gfx/renderers/DefaultRenderer.hpp>
#include <dilithium/math/Math.hpp>
#include <dilithium/platform/Window.hpp>
#include <dilithium/scenes/Scene.hpp>
#include <dilithium/scenes/SceneManager.hpp>
#include <dilithium/scenes/SceneServices.hpp>

#include <memory>
#include <utility>

namespace {

enum class SceneId { ClearScreen };

const char* name(dilithium::AppState state) {
    switch (state) {
    case dilithium::AppState::Active:
        return "active";
    case dilithium::AppState::Inactive:
        return "inactive";
    case dilithium::AppState::Background:
        return "background";
    }
    DILITHIUM_UNREACHABLE("unknown AppState");
}

/// D1: a window that clears to a color whose hue goes once round the color wheel every six seconds, which proves
/// frames advance.
class ClearScreenScene final : public dilithium::Scene {
public:
    explicit ClearScreenScene(dilithium::SceneServices& services) : renderer(services.renderer) {}

    void update(float dt) override {
        hueDegrees = dilithium::math::wrap(hueDegrees + (dt * kDegreesPerSecond), 0.0f, kDegreesPerTurn);
    }

    void draw() override { renderer.setClearColor(dilithium::Color::fromHSV(hueDegrees, kSaturation, kValue)); }

    /// Logged only: D1 has nothing to pause. The engine freezes its loop by itself while the player is away.
    void appStateChanged(dilithium::AppState state) override { dilithium::logInfo("app state: {}", name(state)); }

private:
    static constexpr float kDegreesPerTurn = dilithium::math::kDegreesPerTurn;
    static constexpr float kSecondsPerTurn = 6.0f;
    static constexpr float kDegreesPerSecond = kDegreesPerTurn / kSecondsPerTurn;
    static constexpr float kSaturation = 0.6f; ///< soft, not neon
    static constexpr float kValue = 0.9f;      ///< bright, not white

    dilithium::Renderer& renderer;
    float hueDegrees = 0.0f;
};

} // namespace

std::unique_ptr<dilithium::Engine> dilithium::createEngine(const CommandLine& commandLine) {
    auto window = std::make_unique<Window>(WindowConfig{.title = "D1 Clear Screen"});
    auto renderer = std::make_unique<DefaultRenderer>(*window);
    auto engine = std::make_unique<Engine>(std::move(window), std::move(renderer), commandLine);
    engine->getScenes().add<ClearScreenScene>(SceneId::ClearScreen);
    engine->getScenes().start(SceneId::ClearScreen);
    return engine;
}
