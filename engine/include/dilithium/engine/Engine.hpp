#pragma once

#include <dilithium/platform/AppState.hpp>
#include <dilithium/utilities/NonMovable.hpp>

#include <functional>

namespace dilithium {

class Renderer;
class SceneManager;

/// What one frame asks the shell to do next.
enum class FrameResult {
    Continue, ///< keep calling
    Finished, ///< a `--frames` run ended clean: exit 0
    Failed,   ///< a `--frames` run ended with validation messages: exit 1
};

/// Runs the game: owns the renderer and the scenes, and advances one frame each time the application asks.
/// LiquidMetal2D's `GameEngine` protocol: `DefaultEngine` is the engine's own implementation, and a game that wants
/// another returns it from `Application::createEngine`. The `Application` registers scenes on it and forwards the
/// window's events to it. Nothing here names the window system or the GPU.
class Engine : NonMovable {
public:
    Engine() = default;
    virtual ~Engine() = default; ///< scenes first, while the GPU they use still exists; then the renderer

    [[nodiscard]] virtual SceneManager& getScenes() = 0;
    [[nodiscard]] virtual Renderer& getRenderer() = 0;

    // --- what the application forwards, from the window and the shell

    /// One frame: a pending scene transition, update, draw, present; or nothing, while the loop stands still.
    [[nodiscard]] virtual FrameResult frame() = 0;

    /// A state was reported (by the window, or a phone's lifecycle): delivered to the observers and the scene on
    /// top only when it differs from the last one delivered, then the loop freezes or thaws.
    virtual void appStateReported(AppState state) = 0;

    /// The window's pixel size changed: tell the scene on top. The renderer finds out by itself each frame.
    virtual void resized() = 0;

    /// The player asked to quit: ask the scene on top; quit unless it took the request over.
    virtual void quitRequested() = 0;

    /// Quits for real: the run ends at the start of the next frame.
    virtual void quit() = 0;

    /// The state the game was last told.
    [[nodiscard]] virtual AppState getState() const = 0;

    // --- what the application (or a game, through it) sets

    /// App-level code that wants every `AppState` change, whichever scene is on top. Told before the scene, in the
    /// order added. The `Application` registers its own hook here.
    virtual void addAppStateObserver(std::function<void(AppState)> observer) = 0;

    /// Whether the loop stands still while the app is visible but without focus. On by default. It always stands
    /// still in the background.
    virtual void setPausesWhenInactive(bool pauses) = 0;
};

} // namespace dilithium
