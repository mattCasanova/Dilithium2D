#include "engine/EngineLoop.hpp"

#include "engine/EngineImpl.hpp"

#include <dilithium/core/Assert.hpp>
#include <dilithium/core/Log.hpp>
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
    switch (event.type) {
    case SDL_EVENT_QUIT: // the close button, Cmd-Q, Ctrl-C in the terminal, or SIGTERM
        return SDL_APP_SUCCESS;
    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
        engine.m_impl->resized();
        return SDL_APP_CONTINUE;
    case SDL_EVENT_WINDOW_MINIMIZED:
        DILITHIUM_LOG_DEBUG("window minimized");
        return SDL_APP_CONTINUE;
    case SDL_EVENT_WINDOW_RESTORED:
        DILITHIUM_LOG_DEBUG("window restored");
        return SDL_APP_CONTINUE;
    default:
        // Every other event (keys, mouse, focus, ...) has no reader yet. Input arrives in D9.
        return SDL_APP_CONTINUE;
    }
}

} // namespace dilithium
