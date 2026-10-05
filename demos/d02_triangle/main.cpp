#include "TriangleScene.hpp"

#include <dilithium/engine/Engine.hpp>
#include <dilithium/gfx/renderers/DefaultRenderer.hpp>
#include <dilithium/platform/Window.hpp>
#include <dilithium/scenes/SceneManager.hpp>
#include <dilithium/utilities/CommandLine.hpp>

#include <memory>
#include <utility>

namespace {

enum class SceneId { Triangle };

} // namespace

std::unique_ptr<dilithium::Engine> dilithium::Engine::create(const CommandLine& commandLine) {
    auto window = std::make_unique<Window>(WindowConfig{.title = "D2 Triangle"});
    auto renderer = std::make_unique<DefaultRenderer>(*window);
    auto engine = std::make_unique<Engine>(std::move(window), std::move(renderer), commandLine);
    engine->getScenes().add<demo::TriangleScene>(SceneId::Triangle);
    engine->getScenes().start(SceneId::Triangle);
    return engine;
}
