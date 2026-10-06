#include "ClearScreenScene.hpp"

#include <dilithium/engine/Application.hpp>
#include <dilithium/platform/Window.hpp>
#include <dilithium/scenes/SceneManager.hpp>
#include <dilithium/utilities/CommandLine.hpp>

#include <memory>

namespace {

enum class SceneId { ClearScreen };

class ClearScreenApp final : public dilithium::Application {
public:
    using Application::Application;

protected:
    [[nodiscard]] dilithium::WindowConfig windowConfig() const override { return {.title = "D1 Clear Screen"}; }

    void registerScenes(dilithium::SceneManager& scenes) override {
        scenes.add<demo::ClearScreenScene>(SceneId::ClearScreen);
        scenes.start(SceneId::ClearScreen);
    }
};

} // namespace

std::unique_ptr<dilithium::Application> dilithium::createApplication(const CommandLine& commandLine) {
    return std::make_unique<ClearScreenApp>(commandLine);
}
