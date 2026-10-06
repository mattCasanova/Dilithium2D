#include "Shell.hpp"

#include <dilithium/engine/Application.hpp>
#include <dilithium/engine/Engine.hpp>
#include <dilithium/platform/AppState.hpp>
#include <dilithium/utilities/Assert.hpp>
#include <dilithium/utilities/CommandLine.hpp>
#include <dilithium/utilities/Log.hpp>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <atomic>
#include <csignal>
#include <exception>
#include <format>
#include <memory>
#include <stdexcept>
#include <string_view>
#ifndef DILITHIUM_DEBUG
#include <string>
#endif

namespace dilithium {
namespace {

Application& applicationFrom(void* appstate) {
    DILITHIUM_ASSERT(appstate != nullptr, "the platform called back before init handed over the application");
    return *static_cast<Application*>(appstate);
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

/// Runs one callback's body. An exception must not cross into the platform's C code, so it is logged and ends the
/// program.
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

SDL_AppResult resultFor(FrameResult result) {
    switch (result) {
    case FrameResult::Continue:
        return SDL_APP_CONTINUE;
    case FrameResult::Finished:
        return SDL_APP_SUCCESS;
    case FrameResult::Failed:
        return SDL_APP_FAILURE;
    }
    DILITHIUM_UNREACHABLE("unknown FrameResult");
}

} // namespace

std::atomic<bool> Shell::quitSignalReceived{false};

int Shell::run(int argc, char** argv) {
    return SDL_EnterAppMainCallbacks(argc, argv, &Shell::init, &Shell::iterate, &Shell::event, &Shell::quit);
}

void Shell::onQuitSignal(int /*signal*/) {
    quitSignalReceived.store(true);
}

/// Everything `init` does. Kept out of the callback so `guarded` can wrap it in one line.
SDL_AppResult Shell::startUp(void** appstate, int argc, char** argv) {
    // The close button and Command-Q are requests the scene answers (Scene::quitRequested), never a quit by
    // themselves. Ctrl-C and SIGTERM in a terminal are a developer's deliberate quit and end the run at once: our
    // own handler, not the platform's, which would fold them into the same request.
    SDL_SetHint(SDL_HINT_QUIT_ON_LAST_WINDOW_CLOSE, "0");
    SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");
    std::signal(SIGINT, onQuitSignal);
    std::signal(SIGTERM, onQuitSignal);
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        throw std::runtime_error(std::format("SDL_Init failed: {}", SDL_GetError()));
    }
    const CommandLine commandLine(argc, argv);
    std::unique_ptr<Application> application = createApplication(commandLine);
    if (!application) {
        DILITHIUM_UNREACHABLE("createApplication returned null");
    }
    application->start();
    // appstate is a plain pointer: ownership passes to the platform here and comes back in quit.
    *appstate = application.release();
    return SDL_APP_CONTINUE;
}

SDL_AppResult Shell::init(void** appstate, int argc, char** argv) {
    return guarded("init", [&] { return startUp(appstate, argc, argv); }, /*startingUp=*/true);
}

SDL_AppResult Shell::iterate(void* appstate) {
    return guarded("iterate", [&] {
        if (quitSignalReceived.load()) {
            logInfo("quitting on a signal");
            return SDL_APP_SUCCESS;
        }
        return resultFor(applicationFrom(appstate).frame());
    });
}

SDL_AppResult Shell::event(void* appstate, SDL_Event* event) {
    return guarded("event", [&] { return forward(appstate, *event); });
}

/// The platform's events, mapped to the application's calls. Input (keys, mouse, ...) has no reader yet: D9.
SDL_AppResult Shell::forward(void* appstate, const SDL_Event& event) {
    Application& application = applicationFrom(appstate);
    switch (event.type) {
    case SDL_EVENT_QUIT:                   // Command-Q, the menu's Quit, the Dock's Quit, a logout
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED: // the close button
        application.quitRequested();
        return SDL_APP_CONTINUE;
    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
        application.resized();
        return SDL_APP_CONTINUE;

    // Anything that may change whether the player can see and use the window.
    case SDL_EVENT_WINDOW_FOCUS_GAINED:
    case SDL_EVENT_WINDOW_FOCUS_LOST:
    case SDL_EVENT_WINDOW_MINIMIZED:
    case SDL_EVENT_WINDOW_RESTORED:
    case SDL_EVENT_WINDOW_HIDDEN:
    case SDL_EVENT_WINDOW_SHOWN:
        application.windowChanged();
        return SDL_APP_CONTINUE;

    // A phone says so directly, in LiquidMetal2D's order: resign active, enter background; enter foreground,
    // become active. Untested until there is a phone build.
    case SDL_EVENT_WILL_ENTER_BACKGROUND:
    case SDL_EVENT_WILL_ENTER_FOREGROUND:
        application.getEngine().appStateReported(AppState::Inactive);
        return SDL_APP_CONTINUE;
    case SDL_EVENT_DID_ENTER_BACKGROUND:
        application.getEngine().appStateReported(AppState::Background);
        return SDL_APP_CONTINUE;
    case SDL_EVENT_DID_ENTER_FOREGROUND:
        application.getEngine().appStateReported(AppState::Active);
        return SDL_APP_CONTINUE;

    default:
        return SDL_APP_CONTINUE;
    }
}

void Shell::quit(void* appstate, SDL_AppResult result) {
    // Ownership comes back from the platform. Null when init failed before handing it over, which is normal there.
    const std::unique_ptr<Application> application(static_cast<Application*>(appstate));
    if (!application) {
        return;
    }
    logInfo("shutting down after {}", result == SDL_APP_SUCCESS ? "a clean quit" : "a failure");
    // The application's destructor runs here: scenes, renderer, engine, window. The platform cleans itself up once
    // this returns.
}

} // namespace dilithium

/// The entry point of every Dilithium2D program. Where a platform needs its own entry point (Windows, phones), the
/// platform's header renames this to `SDL_main` and supplies the real one, which calls it.
int main(int argc, char* argv[]) {
    return dilithium::Shell::run(argc, argv);
}
