// SDL's entry point for every Dilithium2D program: the dilithium::main target. SDL owns the main loop and calls the
// four SDL_App* callbacks below, which forward to the Engine the game built in createEngine. A game links this file
// and never includes an SDL header.
#define SDL_MAIN_USE_CALLBACKS 1 // NOLINT(cppcoreguidelines-macro-usage): SDL_main.h reads it as a macro

#include "engine/EngineLoop.hpp"

#include <dilithium/core/Assert.hpp>
#include <dilithium/core/Log.hpp>
#include <dilithium/engine/CommandLine.hpp>
#include <dilithium/engine/Engine.hpp>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <exception>
#include <format>
#include <memory>
#include <stdexcept>
#include <string_view>

namespace {

using dilithium::Engine;
using dilithium::EngineLoop;
using dilithium::logError;
using dilithium::logInfo;

Engine& engineFrom(void* appstate) {
    DILITHIUM_ASSERT(appstate != nullptr, "SDL called back before SDL_AppInit handed over the engine");
    return *static_cast<Engine*>(appstate);
}

/// A start-up failure is shown to the player too in release builds: a game launched from Finder or Explorer has no
/// visible stderr. Debug builds run from a terminal, and a box would block an automated run.
void showStartupFailure([[maybe_unused]] std::string_view message) {
#ifndef DILITHIUM_DEBUG
    const std::string text(message);
    if (!SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Dilithium2D could not start", text.c_str(), nullptr)) {
        logError("could not show the error box: {}", SDL_GetError());
    }
#endif
}

/// Runs one callback's body. An exception must not cross into SDL's C code, so it is logged and ends the program.
template <typename Body>
SDL_AppResult guarded(const char* callback, const Body& body, bool startingUp = false) {
    try {
        return body();
    } catch (const std::exception& error) {
        logError("{}: {}", callback, error.what());
        if (startingUp) {
            showStartupFailure(error.what());
        }
    } catch (...) {
        logError("{}: unknown exception", callback);
        if (startingUp) {
            showStartupFailure("unknown exception");
        }
    }
    return SDL_APP_FAILURE;
}

/// Everything SDL_AppInit does. Kept out of the callback so `guarded` can wrap it in one line.
SDL_AppResult startUp(void** appstate, int argc, char** argv) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        throw std::runtime_error(std::format("SDL_Init failed: {}", SDL_GetError()));
    }
    const dilithium::CommandLine commandLine(argc, argv);
    std::unique_ptr<Engine> engine = dilithium::createEngine(commandLine);
    if (!engine) {
        DILITHIUM_UNREACHABLE("createEngine returned null");
    }
    // appstate is a plain pointer: ownership passes to SDL here and comes back in SDL_AppQuit.
    *appstate = engine.release();
    return SDL_APP_CONTINUE;
}

} // namespace

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
    return guarded("SDL_AppInit", [&] { return startUp(appstate, argc, argv); }, /*startingUp=*/true);
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    return guarded("SDL_AppIterate", [&] { return EngineLoop::iterate(engineFrom(appstate)); });
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    return guarded("SDL_AppEvent", [&] { return EngineLoop::event(engineFrom(appstate), *event); });
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    // Ownership comes back from SDL. Null when SDL_AppInit failed before handing it over, which is normal there.
    const std::unique_ptr<Engine> engine(static_cast<Engine*>(appstate));
    if (!engine) {
        return;
    }
    logInfo("shutting down after {}", result == SDL_APP_SUCCESS ? "a clean quit" : "a failure");
    // The engine's destructor runs here: scenes, then the renderer, then the window. SDL calls SDL_Quit itself
    // once this returns.
}
