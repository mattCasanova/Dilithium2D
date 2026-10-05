#pragma once

#include <dilithium/platform/AppState.hpp>
#include <dilithium/scenes/Scene.hpp>

namespace dilithium {
class Renderer;
struct SceneServices;
} // namespace dilithium

namespace demo {

/// D1: a window that clears to a color whose hue goes once round the color wheel every six seconds, which proves
/// frames advance.
class ClearScreenScene final : public dilithium::Scene {
public:
    explicit ClearScreenScene(dilithium::SceneServices& services);

    void update(float dt) override;
    void draw() override;

    /// Logged only: D1 has nothing to pause. The engine freezes its loop by itself while the player is away.
    void appStateChanged(dilithium::AppState state) override;

private:
    dilithium::Renderer& renderer;
    float hueDegrees = 0.0f;
};

} // namespace demo
