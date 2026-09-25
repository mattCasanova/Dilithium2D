// SDL's entry point for every Dilithium2D program: the dilithium::main target. SDL owns the main loop and calls the
// four SDL_App* callbacks below, which run the game's App. A game links this file and never includes an SDL header.
#define SDL_MAIN_USE_CALLBACKS 1

#include "core/FrameTime.hpp"
#include "platform/Window.hpp"

#include <dilithium/App.hpp>
#include <dilithium/core/Assert.hpp>
#include <dilithium/core/Log.hpp>
#include <dilithium/core/Version.hpp>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <cstdint>
#include <exception>
#include <memory>
#include <utility>

namespace {

using dilithium::App;
using dilithium::logError;
using dilithium::logInfo;
using dilithium::logWarning;
using dilithium::PixelSize;
using dilithium::Window;

/// Everything one run of the program owns, handed to SDL as `appstate`. `app` is declared last, so it is destroyed
/// first, while the window (and, from Phase 5, the GPU) it may use still exist.
struct Runtime {
    Window window;
    std::unique_ptr<App> app;
    uint64_t lastFrameNs = 0;
};

Runtime& runtimeFrom(void* appstate) {
    LOGICAL(appstate != nullptr, "SDL called back before SDL_AppInit handed over the runtime");
    return *static_cast<Runtime*>(appstate);
}

/// Runs one callback's body. An exception must not cross into SDL's C code, so it is logged and ends the program.
template <typename Body>
SDL_AppResult guarded(const char* callback, Body&& body) {
    try {
        return body();
    } catch (const std::exception& error) {
        logError("{}: {}", callback, error.what());
    } catch (...) {
        logError("{}: unknown exception", callback);
    }
    return SDL_APP_FAILURE;
}

} // namespace

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
    return guarded("SDL_AppInit", [&] {
        // Nothing paces the loop until Phase 6 presents frames through FIFO, so iterate only when an event arrives.
        if (!SDL_SetHint(SDL_HINT_MAIN_CALLBACK_RATE, "waitevent")) {
            logWarning("could not set {}: {}", SDL_HINT_MAIN_CALLBACK_RATE, SDL_GetError());
        }
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            logError("SDL_Init failed: {}", SDL_GetError());
            return SDL_APP_FAILURE;
        }

        std::unique_ptr<App> app = dilithium::createApp(argc, argv);
        if (!app) {
            ILLOGICAL("createApp returned null");
        }
        const dilithium::AppConfig config = app->config();
        Window window(config.title, config.width, config.height);
        const PixelSize pixels = window.pixelSize();
        logInfo("Dilithium2D {}: '{}', {}x{} pixels", dilithium::version(), config.title, pixels.width, pixels.height);

        auto runtime = std::make_unique<Runtime>(std::move(window), std::move(app), SDL_GetTicksNS());
        runtime->app->onStart();
        // appstate is a plain pointer: ownership passes to SDL here and comes back in SDL_AppQuit.
        *appstate = runtime.release();
        return SDL_APP_CONTINUE;
    });
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    return guarded("SDL_AppIterate", [&] {
        Runtime& runtime = runtimeFrom(appstate);
        const uint64_t now = SDL_GetTicksNS();
        const float dt = dilithium::frameSeconds(runtime.lastFrameNs, now);
        runtime.lastFrameNs = now;
        runtime.app->onUpdate(dt);
        return SDL_APP_CONTINUE;
    });
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    return guarded("SDL_AppEvent", [&] {
        Runtime& runtime = runtimeFrom(appstate);
        switch (event->type) {
        case SDL_EVENT_QUIT: // the close button, Cmd-Q, or Ctrl-C in the terminal
            return SDL_APP_SUCCESS;
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
            const PixelSize pixels = runtime.window.pixelSize();
            CAPTAINS_LOG("window is {}x{} pixels", pixels.width, pixels.height);
            return SDL_APP_CONTINUE;
        }
        case SDL_EVENT_WINDOW_MINIMIZED:
            CAPTAINS_LOG("window minimized");
            return SDL_APP_CONTINUE;
        case SDL_EVENT_WINDOW_RESTORED:
            CAPTAINS_LOG("window restored");
            return SDL_APP_CONTINUE;
        default:
            // Every other event (keys, mouse, focus, ...) has no reader yet. Input arrives in D9.
            return SDL_APP_CONTINUE;
        }
    });
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    // Ownership comes back from SDL. Null when SDL_AppInit failed before handing it over, which is normal there.
    const std::unique_ptr<Runtime> runtime(static_cast<Runtime*>(appstate));
    if (!runtime) {
        return;
    }
    try {
        runtime->app->onShutdown();
    } catch (const std::exception& error) {
        logError("SDL_AppQuit: {}", error.what());
    } catch (...) {
        logError("SDL_AppQuit: unknown exception");
    }
    logInfo("shutting down after {}", result == SDL_APP_SUCCESS ? "a clean quit" : "a failure");
    // SDL calls SDL_Quit itself once this returns.
}
