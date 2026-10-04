// SDL's entry point for every Dilithium2D program: the dilithium::main target. SDL owns the main loop and calls the
// four SDL_App* callbacks below, which run the game's App. A game links this file and never includes an SDL header.
#define SDL_MAIN_USE_CALLBACKS 1 // NOLINT(cppcoreguidelines-macro-usage): SDL_main.h reads it as a macro

#include "engine/FrameTime.hpp"
#include "engine/RunOptions.hpp"
#include "engine/Torture.hpp"
#include "gfx/renderers/RenderCore.hpp"
#include "platform/Window.hpp"

#include <dilithium/App.hpp>
#include <dilithium/core/Assert.hpp>
#include <dilithium/core/Log.hpp>
#include <dilithium/core/Version.hpp>
#include <dilithium/gfx/Renderer.hpp>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <cstdint>
#include <exception>
#include <format>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

using dilithium::App;
using dilithium::FrameOutcome;
using dilithium::logError;
using dilithium::logInfo;
using dilithium::PixelSize;
using dilithium::RenderCore;
using dilithium::RunOptions;
using dilithium::TortureAction;
using dilithium::TortureStep;
using dilithium::Window;

/// How a run's frames went, for the summary a `--frames` run ends with.
struct FrameCounts {
    uint64_t presented = 0;
    uint64_t skipped = 0;
    uint64_t idle = 0;
};

/// Everything one run of the program owns, handed to SDL as `appstate`. Declared in creation order, so it is
/// destroyed the other way round: the app first, then the GPU, then the window the GPU draws into.
struct Runtime {
    Runtime(Window newWindow, std::unique_ptr<App> newApp, RunOptions runOptions)
        : window(std::move(newWindow)), renderCore(window), app(std::move(newApp)), options(runOptions),
          lastFrameNs(SDL_GetTicksNS()) {}

    Window window;
    RenderCore renderCore;
    std::unique_ptr<App> app;
    RunOptions options;
    uint64_t frame = 0; ///< loop ticks so far, presented or not
    FrameCounts counts;
    uint64_t lastFrameNs;
};

/// How long to sleep when no frame was presented (minimized, or no area). FIFO present paces every other frame; this
/// keeps an idle window from spinning a CPU core while still noticing a restore within a frame or so.
constexpr uint32_t kIdleSleepMs = 16;

Runtime& runtimeFrom(void* appstate) {
    DILITHIUM_ASSERT(appstate != nullptr, "SDL called back before SDL_AppInit handed over the runtime");
    return *static_cast<Runtime*>(appstate);
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

void applyTorture(Window& window, TortureStep step) {
    switch (step.action) {
    case TortureAction::None:
        return;
    case TortureAction::Resize:
        window.resize(step.width, step.height);
        return;
    case TortureAction::Minimize:
        window.minimize();
        return;
    case TortureAction::Restore:
        window.restore();
        return;
    }
    DILITHIUM_UNREACHABLE("unknown TortureAction");
}

void count(FrameCounts& counts, FrameOutcome outcome) {
    switch (outcome) {
    case FrameOutcome::Presented:
        ++counts.presented;
        return;
    case FrameOutcome::Skipped:
        ++counts.skipped;
        return;
    case FrameOutcome::Idle:
        ++counts.idle;
        return;
    }
    DILITHIUM_UNREACHABLE("unknown FrameOutcome");
}

/// The end of a `--frames` run: a summary line, and failure if validation said anything, so a run cannot pass by
/// luck. (Errors already aborted; this catches warnings.)
SDL_AppResult finishRun(const Runtime& runtime) {
    const FrameCounts& counts = runtime.counts;
    logInfo("ran {} frames: {} presented, {} skipped, {} idle; {} swapchain builds", runtime.frame, counts.presented,
            counts.skipped, counts.idle, runtime.renderCore.swapchainBuilds());
    const uint32_t messages = runtime.renderCore.validationMessages();
    if (messages > 0) {
        logError("validation reported {} messages during the run", messages);
        return SDL_APP_FAILURE;
    }
    return SDL_APP_SUCCESS;
}

/// Everything SDL_AppInit does. Kept out of the callback so `guarded` can wrap it in one line.
SDL_AppResult startUp(void** appstate, int argc, char** argv) {
    const std::vector<std::string_view> args(argv, argv + argc);
    const RunOptions options = dilithium::parseRunOptions(args);
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        throw std::runtime_error(std::format("SDL_Init failed: {}", SDL_GetError()));
    }

    std::unique_ptr<App> app = dilithium::createApp(argc, argv);
    if (!app) {
        DILITHIUM_UNREACHABLE("createApp returned null");
    }
    const dilithium::AppConfig config = app->config();
    Window window(config.title, config.width, config.height);
    const PixelSize pixels = window.pixelSize();
    logInfo("Dilithium2D {}: '{}', {}x{} pixels", dilithium::version(), config.title, pixels.width, pixels.height);

    if (options.frames || options.torture) {
        logInfo("run: {}{}", options.frames ? std::format("{} frames", *options.frames) : "until quit",
                options.torture ? ", with torture" : "");
    }

    auto runtime = std::make_unique<Runtime>(std::move(window), std::move(app), options);
    runtime->app->onStart();
    // appstate is a plain pointer: ownership passes to SDL here and comes back in SDL_AppQuit.
    *appstate = runtime.release();
    return SDL_APP_CONTINUE;
}

} // namespace

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
    return guarded("SDL_AppInit", [&] { return startUp(appstate, argc, argv); }, /*startingUp=*/true);
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    return guarded("SDL_AppIterate", [&] {
        Runtime& runtime = runtimeFrom(appstate);
        ++runtime.frame;
        if (runtime.options.torture) {
            applyTorture(runtime.window, dilithium::tortureStep(runtime.frame));
        }

        const uint64_t now = SDL_GetTicksNS();
        const float dt = dilithium::frameSeconds(runtime.lastFrameNs, now);
        runtime.lastFrameNs = now;
        runtime.app->onUpdate(dt);

        const FrameOutcome outcome = runtime.renderCore.drawFrame(runtime.window, runtime.app->clearColor());
        count(runtime.counts, outcome);
        if (outcome == FrameOutcome::Idle) {
            SDL_Delay(kIdleSleepMs);
        }

        if (runtime.options.frames && runtime.frame >= *runtime.options.frames) {
            return finishRun(runtime);
        }
        return SDL_APP_CONTINUE;
    });
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    return guarded("SDL_AppEvent", [&] {
        const Runtime& runtime = runtimeFrom(appstate);
        switch (event->type) {
        case SDL_EVENT_QUIT: // the close button, Cmd-Q, Ctrl-C in the terminal, or SIGTERM
            return SDL_APP_SUCCESS;
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
            const PixelSize pixels = runtime.window.pixelSize();
            // Logged only: drawFrame compares the window's pixel size with the swapchain's every frame, which also
            // covers a resize whose event has not arrived yet.
            DILITHIUM_LOG_DEBUG("window is {}x{} pixels", pixels.width, pixels.height);
            return SDL_APP_CONTINUE;
        }
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
