#pragma once

namespace dilithium {

/// Whether the player can see and use the game. The engine tracks it from the window system and tells the game
/// when it changes (`Scene::appStateChanged`, `Engine::addAppStateObserver`); what to do about it, such as pushing a
/// pause scene or saving, is the game's choice. The engine only freezes its own loop while the player is away.
enum class AppState {
    Active,     ///< in front with focus: the player is playing
    Inactive,   ///< visible but without focus: Command-Tab to another app, another window on top
    Background, ///< not visible: minimized, hidden, or the phone's home screen. On a phone, save here
};

/// What a scene may ask of the program as a whole, through `SceneServices::app`. LiquidMetal2D's `LiquidApp`, as
/// an interface instead of a global, so a test can pass a fake. The `Engine` implements it.
class Application {
public:
    Application() = default;
    virtual ~Application() = default;

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(Application&&) = delete;

    /// Quits for real: the run ends at the start of the next frame with the normal teardown (scenes, then the
    /// renderer, then the window). The engine never quits on its own; it asks the current scene first
    /// (`Scene::quitRequested`), and a scene that took the request over calls this when it is ready.
    virtual void quit() = 0;

    /// The app's state as of the latest change the window system reported.
    [[nodiscard]] virtual AppState state() const = 0;
};

} // namespace dilithium
