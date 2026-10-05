#pragma once

#include <dilithium/platform/AppState.hpp>
#include <dilithium/scenes/Scene.hpp>

namespace dilithium {
class Renderer;
class Window;
struct SceneServices;
} // namespace dilithium

namespace stress {

/// Drives the window through the schedule while drawing what the demos draw: a clear whose hue cycles and a turning
/// triangle. Each resize rebuilds the swapchain and each minimize freezes the loop; if validation stays silent
/// through 600 frames of it, those paths hold.
class StressScene final : public dilithium::Scene {
public:
    StressScene(dilithium::SceneServices& services, dilithium::Window& stressed);

    void update(float dt) override;
    void draw() override;

    /// Minimized is `Background`, and the engine stands still there: no `update` will come to restore the window,
    /// so the restore is asked for here, as soon as the engine says so.
    void appStateChanged(dilithium::AppState state) override;

private:
    dilithium::Renderer& renderer;
    dilithium::Window& window;
    int frame = 0;
    float hueDegrees = 0.0f;
    float angle = 0.0f;
};

} // namespace stress
