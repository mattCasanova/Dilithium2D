#include "EngineLoop.hpp"

#include "engine/AppStateTracker.hpp"
#include "engine/EngineImpl.hpp"

#include <dilithium/core/Assert.hpp>
#include <dilithium/engine/Application.hpp>
#include <dilithium/engine/Engine.hpp>

#include <SDL3/SDL.h>

namespace dilithium {

SDL_AppResult EngineLoop::iterate(Engine& engine) {
    switch (engine.m_impl->frame()) {
    case FrameResult::Continue:
        return SDL_APP_CONTINUE;
    case FrameResult::Finished:
        return SDL_APP_SUCCESS;
    case FrameResult::Failed:
        return SDL_APP_FAILURE;
    }
    DILITHIUM_UNREACHABLE("unknown FrameResult");
}

SDL_AppResult EngineLoop::event(Engine& engine, const SDL_Event& event) {
    Engine::Impl& impl = *engine.m_impl;
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

    // The facts the app state follows from, on a desktop.
    case SDL_EVENT_WINDOW_FOCUS_GAINED:
    case SDL_EVENT_WINDOW_FOCUS_LOST:
    case SDL_EVENT_WINDOW_MINIMIZED:
    case SDL_EVENT_WINDOW_RESTORED:
    case SDL_EVENT_WINDOW_HIDDEN:
    case SDL_EVENT_WINDOW_SHOWN: {
        WindowFacts facts = impl.windowFacts;
        switch (event.type) {
        case SDL_EVENT_WINDOW_FOCUS_GAINED:
            facts.focused = true;
            break;
        case SDL_EVENT_WINDOW_FOCUS_LOST:
            facts.focused = false;
            break;
        case SDL_EVENT_WINDOW_MINIMIZED:
            facts.minimized = true;
            break;
        case SDL_EVENT_WINDOW_RESTORED:
            facts.minimized = false;
            break;
        case SDL_EVENT_WINDOW_HIDDEN:
            facts.hidden = true;
            break;
        default: // SDL_EVENT_WINDOW_SHOWN; the outer switch admits nothing else here
            facts.hidden = false;
            break;
        }
        impl.windowFactsChanged(facts);
        return SDL_APP_CONTINUE;
    }

    // A phone says so directly, in LiquidMetal2D's order: resign active, enter background; enter foreground,
    // become active. Untested until there is a phone build.
    case SDL_EVENT_WILL_ENTER_BACKGROUND:
    case SDL_EVENT_WILL_ENTER_FOREGROUND:
        impl.appStateSet(AppState::Inactive);
        return SDL_APP_CONTINUE;
    case SDL_EVENT_DID_ENTER_BACKGROUND:
        impl.appStateSet(AppState::Background);
        return SDL_APP_CONTINUE;
    case SDL_EVENT_DID_ENTER_FOREGROUND:
        impl.appStateSet(AppState::Active);
        return SDL_APP_CONTINUE;

    default:
        // Every other event (keys, mouse, ...) has no reader yet. Input arrives in D9.
        return SDL_APP_CONTINUE;
    }
}

} // namespace dilithium
