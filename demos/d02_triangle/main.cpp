#include "TriangleScene.hpp"

#include <dilithium/engine/Application.hpp>
#include <dilithium/platform/Window.hpp>
#include <dilithium/scenes/SceneManager.hpp>
#include <dilithium/utilities/CommandLine.hpp>

#include <memory>

namespace {

enum class SceneId { Triangle };

class TriangleApp final : public dilithium::Application {
public:
    using Application::Application;

protected:
    [[nodiscard]] dilithium::WindowConfig windowConfig() const override { return {.title = "D2 Triangle"}; }

    void registerScenes(dilithium::SceneManager& scenes) override {
        scenes.add<demo::TriangleScene>(SceneId::Triangle);
        scenes.start(SceneId::Triangle);
    }
};

} // namespace

std::unique_ptr<dilithium::Application> dilithium::createApplication(const CommandLine& commandLine) {
    return std::make_unique<TriangleApp>(commandLine);
}
