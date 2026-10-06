#pragma once

#include <dilithium/platform/AppState.hpp>
#include <dilithium/utilities/NonMovable.hpp>

namespace dilithium {

/// What a scene may ask of the program as a whole, through `SceneServices::app`: the `Application`, seen through
/// the two calls a scene needs. LiquidMetal2D's `LiquidApp`, as an interface instead of a global, so a test can pass
/// a fake.
class AppServices : NonMovable {
public:
    AppServices() = default;
    virtual ~AppServices() = default;

    /// Quits for real: the run ends at the start of the next frame with the normal teardown (scenes, then the
    /// renderer, then the window). The engine never quits on its own; it asks the current scene first
    /// (`Scene::quitRequested`), and a scene that took the request over calls this when it is ready.
    virtual void quit() = 0;

    /// The app's state as of the latest change the window reported.
    [[nodiscard]] virtual AppState getState() const = 0;
};

} // namespace dilithium
