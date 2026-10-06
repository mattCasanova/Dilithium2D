#pragma once

#include <dilithium/engine/AppServices.hpp>
#include <dilithium/engine/Engine.hpp>
#include <dilithium/platform/AppState.hpp>
#include <dilithium/platform/Window.hpp>
#include <dilithium/utilities/CommandLine.hpp>

#include <memory>

namespace dilithium {

class Renderer;
class SceneManager;

/// The program, as the game sees it: the top object, LiquidMetal2D's `LiquidViewController`. A game subclasses it,
/// overrides the hooks it needs (at least `registerScenes`), and defines `createApplication` to build it. The
/// shell creates it, calls `start()` once, then forwards every event and frame to it, and destroys it at quit.
///
/// It owns the window and the engine; the engine owns the renderer and the scenes. `start()` builds them in that
/// order through the hooks, so teardown is the reverse with no code for it: scenes, renderer, engine, window. It
/// also is what scenes see as `services.app` (`AppServices`). Nothing here names the window system or the GPU.
class Application : public AppServices {
public:
    explicit Application(
        CommandLine commandLine); ///< keeps a copy: views into argv, which lives as long as the program
    ~Application() override;

    /// Builds everything, in order: the window (`windowConfig`), the renderer (`createRenderer`), the engine
    /// (`createEngine`), then the scenes (`registerScenes`). Once, by the shell, before the first frame. Throws
    /// `std::runtime_error` when any of them cannot be made.
    void start();

    // --- what the shell calls. Plain and public: a game has no reason to, and nothing breaks if it does.

    /// One frame: what the engine does with it, and what the shell should do next.
    [[nodiscard]] FrameResult frame();
    /// Something about the window changed (focus, minimized, hidden): the window says which state that is.
    void windowChanged();
    /// The window's pixel size changed.
    void resized();
    /// The player asked to quit (the close button, Command-Q, the Dock): the scene on top decides.
    void quitRequested();

    // --- AppServices, for scenes
    void quit() override;
    [[nodiscard]] AppState getState() const override;

    // --- what a game may call, from its subclass

    /// Whether the loop stands still while the app is visible but without focus. On by default; off for a game
    /// that must keep running behind another app. It always stands still in the background.
    void setPausesWhenInactive(bool pauses);

    /// The program's arguments, for a game's own flags; the engine's `--frames` is read already.
    [[nodiscard]] const CommandLine& getCommandLine() const { return commandLine; }
    [[nodiscard]] Window& getWindow();
    [[nodiscard]] Engine& getEngine();
    [[nodiscard]] SceneManager& getScenes();

protected:
    // --- the hooks, in the order `start()` calls them

    /// The window to open. The default is the engine's default size and title.
    [[nodiscard]] virtual WindowConfig windowConfig() const;

    /// The renderer to draw with, built for `window`. The default is the engine's own `DefaultRenderer`.
    [[nodiscard]] virtual std::unique_ptr<Renderer> createRenderer(const Window& window);

    /// The engine to run with, owning `renderer`. The default is `DefaultEngine`; override to run your own.
    [[nodiscard]] virtual std::unique_ptr<Engine> createEngine(std::unique_ptr<Renderer> renderer);

    /// Register the scenes with `scenes.add<MyScene>(MyId::Menu)` and `scenes.start` the first.
    virtual void registerScenes(SceneManager& scenes) = 0;

    /// The app's state changed: the player switched away, hid the game, or came back. Told before the scene on top
    /// (`Scene::appStateChanged`), as LiquidMetal2D tells the view controller first. The default does nothing; a game
    /// pushes its pause scene here, once, rather than in every scene.
    virtual void appStateChanged(AppState state);

private:
    CommandLine commandLine; ///< a copy: views into argv, which lives as long as the program
    std::unique_ptr<Window> window;
    std::unique_ptr<Engine> engine;
    bool pausesWhenInactive = true;
};

/// Defined by the game, once per executable: `return std::make_unique<MyGame>(commandLine);`. The shell calls it
/// at start-up, then `start()`.
std::unique_ptr<Application> createApplication(const CommandLine& commandLine);

} // namespace dilithium
