#pragma once

#include <dilithium/engine/Engine.hpp>
#include <dilithium/platform/AppState.hpp>

#include <functional>
#include <memory>

namespace dilithium {

class AppServices;
class CommandLine;
class Renderer;
class SceneManager;

/// The engine's own `Engine`, LiquidMetal2D's `DefaultEngine`: the frame clock, the frame counts, the `--frames`
/// limit, the freeze rule while the player is away. What `Application::createEngine` returns unless a game
/// overrides it.
class DefaultEngine final : public Engine {
public:
    /// Takes ownership of the renderer. `app` is what scenes see as `services.app`. Reads the engine's own flag
    /// (`--frames N`) from the command line; throws `std::invalid_argument` on a bad one.
    DefaultEngine(std::unique_ptr<Renderer> renderer, AppServices& app, const CommandLine& commandLine);
    ~DefaultEngine() override;

    [[nodiscard]] SceneManager& getScenes() override;
    [[nodiscard]] Renderer& getRenderer() override;
    [[nodiscard]] FrameResult frame() override;
    void appStateReported(AppState state) override;
    void resized() override;
    void quitRequested() override;
    void quit() override;
    [[nodiscard]] AppState getState() const override;
    void addAppStateObserver(std::function<void(AppState)> observer) override;
    void setPausesWhenInactive(bool pauses) override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace dilithium
