#include <dilithium/engine/CommandLine.hpp>
#include <dilithium/engine/Engine.hpp>
#include <dilithium/gfx/Color.hpp>
#include <dilithium/gfx/Renderer.hpp>
#include <dilithium/gfx/renderers/DefaultRenderer.hpp>
#include <dilithium/math/Math.hpp>
#include <dilithium/math/Types.hpp>
#include <dilithium/platform/Window.hpp>
#include <dilithium/scenes/Scene.hpp>
#include <dilithium/scenes/SceneManager.hpp>
#include <dilithium/scenes/SceneServices.hpp>

#include <cmath>
#include <memory>
#include <utility>

namespace {

enum class SceneId { Triangle };

/// D2: one triangle, a corner each of red, green and blue, turning once every ten seconds. Red starts at the top:
/// if it is at the bottom, +Y is pointing the wrong way. Turning proves frames advance and that the vertex buffer
/// is rewritten every frame without the GPU reading a half-written one.
class TriangleScene final : public dilithium::Scene {
public:
    explicit TriangleScene(dilithium::SceneServices& services) : renderer(services.renderer) {}

    void update(float dt) override {
        angle = dilithium::math::wrap(angle + (dt * kRadiansPerSecond), 0.0f, dilithium::math::kTwoPi);
    }

    void draw() override {
        renderer.setClearColor(kBackground);
        renderer.drawTriangle(corner(0), corner(1), corner(2), dilithium::colors::kRed, dilithium::colors::kGreen,
                              dilithium::colors::kBlue);
    }

private:
    static constexpr float kSecondsPerTurn = 10.0f;
    static constexpr float kRadiansPerSecond = dilithium::math::kTwoPi / kSecondsPerTurn;
    static constexpr float kRadius = 0.6f; ///< in clip space: 0.6 of the way from the center to the edge
    static constexpr int kCorners = 3;
    static constexpr dilithium::Color kBackground{0.1f, 0.1f, 0.12f, 1.0f};

    /// The corners sit on a circle, a third of a turn apart, the first straight up before any turning.
    [[nodiscard]] dilithium::Vec2 corner(int index) const {
        const float at = angle + dilithium::math::kHalfPi + (static_cast<float>(index) * kThirdTurn);
        return {kRadius * std::cos(at), kRadius * std::sin(at)};
    }
    static constexpr float kThirdTurn = dilithium::math::kTwoPi / kCorners;

    dilithium::Renderer& renderer;
    float angle = 0.0f;
};

} // namespace

std::unique_ptr<dilithium::Engine> dilithium::Engine::create(const CommandLine& commandLine) {
    auto window = std::make_unique<Window>(WindowConfig{.title = "D2 Triangle"});
    auto renderer = std::make_unique<DefaultRenderer>(*window);
    auto engine = std::make_unique<Engine>(std::move(window), std::move(renderer), commandLine);
    engine->getScenes().add<TriangleScene>(SceneId::Triangle);
    engine->getScenes().start(SceneId::Triangle);
    return engine;
}
