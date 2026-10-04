#pragma once

#include <dilithium/engine/Application.hpp>

#include <functional>
#include <memory>

namespace dilithium {

class CommandLine;
class Renderer;
class SceneManager;
class Window;

/// Runs the game: owns the window and the renderer the game built, the scenes, the frame clock and the frame counts,
/// and advances one frame each time the platform's loop asks. LiquidMetal2D's `DefaultEngine`. The game creates it
/// in `createEngine`, registers its scenes, names the first, and returns it; the platform's entry point
/// (`dilithium::main`) drives it from there and destroys it at quit. Nothing here names the window system or the GPU.
class Engine {
public:
    /// Takes ownership of both. The renderer must have been built for that window. Reads the engine's own flags
    /// from the command line; throws `std::invalid_argument` on a bad one.
    Engine(std::unique_ptr<Window> window, std::unique_ptr<Renderer> renderer, const CommandLine& commandLine);
    ~Engine(); ///< scenes first, while the GPU and window they use still exist; then the renderer; then the window

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;
    Engine(Engine&&) = delete;
    Engine& operator=(Engine&&) = delete;

    /// Register scenes here and `start` the first, before returning from `createEngine`.
    [[nodiscard]] SceneManager& scenes();

    /// App-level code that wants every `AppState` change, whichever scene is on top (a save service). Told before
    /// the current scene, in the order added.
    void addAppStateObserver(std::function<void(AppState)> observer);

    /// Whether the loop stands still while the app is visible but without focus. On by default; off for a game
    /// that must keep running behind another app. It always stands still in the background.
    void setPausesWhenInactive(bool pauses);

private:
    friend class EngineLoop; ///< the entry point's only way in: one frame, one event

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

/// Defined by the game, once per executable: build a `Window`, a `Renderer` and the `Engine`, register scenes, start
/// the first. The engine's entry point calls it at start-up.
std::unique_ptr<Engine> createEngine(const CommandLine& commandLine);

} // namespace dilithium
