#pragma once

namespace dilithium {

/// Whether the player can see and use the game, as the window reports it: hidden or minimized is `Background`;
/// otherwise focus decides between `Active` and `Inactive`. The engine tells the game when it changes
/// (`Scene::appStateChanged`, `Engine::addAppStateObserver`); what to do about it, such as pushing a pause scene or
/// saving, is the game's choice. The engine only freezes its own loop while the player is away.
enum class AppState {
    Active,     ///< in front with focus: the player is playing
    Inactive,   ///< visible but without focus: Command-Tab to another app, another window on top
    Background, ///< not visible: minimized, hidden, or the phone's home screen. On a phone, save here
};

} // namespace dilithium
