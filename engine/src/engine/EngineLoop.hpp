#pragma once

#include <SDL3/SDL.h>

namespace dilithium {

class Engine;

/// The entry point's only way into the engine: one frame, one event, each mapped to SDL's answer. `Engine` names
/// it a friend; nothing else reaches `Engine::Impl`.
class EngineLoop {
public:
    static SDL_AppResult iterate(Engine& engine);
    static SDL_AppResult event(Engine& engine, const SDL_Event& event);
};

} // namespace dilithium
