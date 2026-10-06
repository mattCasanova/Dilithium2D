#pragma once

#include <SDL3/SDL.h>

#include <atomic>

namespace dilithium {

/// The shell: the platform's side of every Dilithium2D program, LiquidMetal2D's `App` and `GameWindow`. The platform
/// owns the main loop and calls the four callbacks below once each at start-up and quit, and once per event and per
/// frame in between; they build the game's `Application` (`createApplication`), forward everything to it, and
/// destroy it at quit. Terminal signals are a developer's deliberate quit and end the run at once, before the game
/// hears anything. Nothing a game includes reaches this header.
class Shell {
public:
    /// What `main` (at the bottom of `Shell.cpp`) calls. Returns the process exit code.
    static int run(int argc, char** argv);

private:
    static SDL_AppResult init(void** appstate, int argc, char** argv);
    static SDL_AppResult iterate(void* appstate);
    static SDL_AppResult event(void* appstate, SDL_Event* event);
    static void quit(void* appstate, SDL_AppResult result);

    static SDL_AppResult startUp(void** appstate, int argc, char** argv);
    static SDL_AppResult forward(void* appstate, const SDL_Event& event);
    static void onQuitSignal(int signal);

    /// Set by the signal handler, read once per frame. A signal handler can reach nothing else, and an atomic is the
    /// only thing it may touch.
    static std::atomic<bool> quitSignalReceived;
};

} // namespace dilithium
