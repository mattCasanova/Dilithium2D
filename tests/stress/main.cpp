// The stress run: `stress_run --frames 600` resizes the window every 20 frames and minimizes it every 97 while
// drawing, and exits 0 only if validation said nothing. The engine's proof after every change to it.
#include "StressScene.hpp"

#include <dilithium/engine/CommandLine.hpp>
#include <dilithium/engine/Engine.hpp>
#include <dilithium/gfx/renderers/DefaultRenderer.hpp>
#include <dilithium/platform/Window.hpp>
#include <dilithium/scenes/Scene.hpp>
#include <dilithium/scenes/SceneManager.hpp>
#include <dilithium/scenes/SceneServices.hpp>

#include <memory>
#include <utility>

namespace {

enum class SceneId { Stress };

} // namespace

std::unique_ptr<dilithium::Engine> dilithium::createEngine(const CommandLine& commandLine) {
    auto window = std::make_unique<Window>(WindowConfig{.title = "Dilithium2D stress run"});
    Window& stressWindow = *window; // the engine owns it and outlives every scene
    auto renderer = std::make_unique<DefaultRenderer>(*window);
    auto engine = std::make_unique<Engine>(std::move(window), std::move(renderer), commandLine);
    engine->getScenes().add(SceneId::Stress, [&stressWindow](SceneServices& services) -> std::unique_ptr<Scene> {
        return std::make_unique<stress::StressScene>(services, stressWindow);
    });
    engine->getScenes().start(SceneId::Stress);
    return engine;
}
