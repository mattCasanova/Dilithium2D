#pragma once

#include <dilithium/engine/Application.hpp>

namespace dilithium {

/// A scene's answer to `Scene::quitRequested`.
enum class QuitResponse {
    Quit,    ///< quit now, with the normal teardown
    Handled, ///< the scene takes it from here: a prompt, a save, then `Application::quit()` when ready
};

/// One screen or state of the game: a menu, the play field, a pause overlay. The game subclasses it and registers
/// the subclass with `SceneManager::add`; the manager builds it with `SceneType(SceneServices&)` and destroys it when
/// it is replaced or popped. The constructor is the setup and the destructor the shutdown, so there is no `init` or
/// `shutdown` to forget; the hooks below are for what is neither.
class Scene {
public:
    Scene() = default;
    virtual ~Scene() = default;

    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;
    Scene(Scene&&) = delete;
    Scene& operator=(Scene&&) = delete;

    /// Once per frame. `dt` is in seconds, clamped so a stall (a breakpoint, a window drag) moves the game one step.
    virtual void update(float dt) = 0;

    /// Once per frame, after `update`: record into the `Renderer` what this frame shows. The engine runs the frame
    /// around it; a scene never begins or ends a pass.
    virtual void draw() = 0;

    /// The window's pixel size changed. Also called after a pop, since the scene below may have missed one.
    virtual void resize() {}

    /// Back on top: the scene pushed over this one was popped.
    virtual void resume() {}

    /// The player asked to quit: the close button, Command-Q, the Dock's Quit, a logout. The engine never quits on
    /// its own; it asks the scene on top. The default quits at once, so a game with no handler is still quittable.
    /// Return `Handled` to take over: show a prompt, save, then call `services.app.quit()` when ready.
    virtual QuitResponse quitRequested() { return QuitResponse::Quit; }

    /// The app's state changed: the player switched away, hid the game, or came back. Only the scene on top hears
    /// it, after the engine's app-level observers. A transition asked for here (pushing a pause scene) happens at
    /// once and is drawn once, so it is on screen while the player is away; the engine then freezes its loop. The
    /// default does nothing. Nothing pops on return: the player dismisses the pause menu.
    virtual void appStateChanged(AppState /*state*/) {}
};

} // namespace dilithium
