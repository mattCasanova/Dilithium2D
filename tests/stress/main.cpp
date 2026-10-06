// The stress run: `stress_run --frames 600` resizes the window every 20 frames and minimizes it every 97 while
// drawing, and exits 0 only if validation said nothing. The engine's proof after every change to it.
#include "StressScene.hpp"

#include <dilithium/engine/Application.hpp>
#include <dilithium/platform/Window.hpp>
#include <dilithium/scenes/Scene.hpp>
#include <dilithium/scenes/SceneManager.hpp>
#include <dilithium/scenes/SceneServices.hpp>
#include <dilithium/utilities/CommandLine.hpp>

#include <memory>

namespace {

enum class SceneId { Stress };

class StressApp final : public dilithium::Application {
public:
    using Application::Application;

protected:
    [[nodiscard]] dilithium::WindowConfig windowConfig() const override { return {.title = "Dilithium2D stress run"}; }

    /// The scene drives the window, which the application owns and which outlives every scene.
    void registerScenes(dilithium::SceneManager& scenes) override {
        dilithium::Window& window = getWindow();
        scenes.add(SceneId::Stress, [&window](dilithium::SceneServices& services) -> std::unique_ptr<dilithium::Scene> {
            return std::make_unique<stress::StressScene>(services, window);
        });
        scenes.start(SceneId::Stress);
    }
};

} // namespace

std::unique_ptr<dilithium::Application> dilithium::createApplication(const CommandLine& commandLine) {
    return std::make_unique<StressApp>(commandLine);
}
