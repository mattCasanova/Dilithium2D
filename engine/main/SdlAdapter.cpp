#include "SdlAdapter.hpp"

#include "engine/EngineImpl.hpp"

#include <dilithium/engine/Engine.hpp>
#include <dilithium/platform/AppState.hpp>
#include <dilithium/utilities/Assert.hpp>

#include <SDL3/SDL.h>

namespace dilithium {

SDL_AppResult SdlAdapter::iterate(Engine& engine) {
    switch (engine.impl->frame()) {
    case FrameResult::Continue:
        return SDL_APP_CONTINUE;
    case FrameResult::Finished:
        return SDL_APP_SUCCESS;
    case FrameResult::Failed:
        return SDL_APP_FAILURE;
    }
    DILITHIUM_UNREACHABLE("unknown FrameResult");
}

SDL_AppResult SdlAdapter::event(Engine& engine, const SDL_Event& event) {
    Engine::Impl& impl = *engine.impl;
    switch (event.type) {
    case SDL_EVENT_QUIT:                   // Command-Q, the menu's Quit, the Dock's Quit, a logout
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED: // the close button
        // A request, not an order: the scene on top decides (Scene::quitRequested). Terminal signals never
        // arrive here; SdlMain handles them itself.
        impl.quitRequested();
        return SDL_APP_CONTINUE;
    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
        impl.resized();
        return SDL_APP_CONTINUE;

    // Anything that may change whether the player can see and use the window. The window reads its own flags and
    // says which state that is; the engine tells the game only when the answer changed.
    case SDL_EVENT_WINDOW_FOCUS_GAINED:
    case SDL_EVENT_WINDOW_FOCUS_LOST:
    case SDL_EVENT_WINDOW_MINIMIZED:
    case SDL_EVENT_WINDOW_RESTORED:
    case SDL_EVENT_WINDOW_HIDDEN:
    case SDL_EVENT_WINDOW_SHOWN:
        impl.windowStateChanged();
        return SDL_APP_CONTINUE;

    // A phone says so directly, in LiquidMetal2D's order: resign active, enter background; enter foreground,
    // become active. Untested until there is a phone build.
    case SDL_EVENT_WILL_ENTER_BACKGROUND:
    case SDL_EVENT_WILL_ENTER_FOREGROUND:
        impl.appStateReported(AppState::Inactive);
        return SDL_APP_CONTINUE;
    case SDL_EVENT_DID_ENTER_BACKGROUND:
        impl.appStateReported(AppState::Background);
        return SDL_APP_CONTINUE;
    case SDL_EVENT_DID_ENTER_FOREGROUND:
        impl.appStateReported(AppState::Active);
        return SDL_APP_CONTINUE;

    default:
        // Every other event (keys, mouse, ...) has no reader yet. Input arrives in D9.
        return SDL_APP_CONTINUE;
    }
}

} // namespace dilithium
