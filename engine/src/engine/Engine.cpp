#include "engine/AppStateTracker.hpp"
#include "engine/EngineImpl.hpp"
#include "engine/FrameClock.hpp"
#include "engine/RunOptions.hpp"

#include <dilithium/core/Assert.hpp>
#include <dilithium/core/Log.hpp>
#include <dilithium/core/Version.hpp>
#include <dilithium/engine/Application.hpp>
#include <dilithium/engine/CommandLine.hpp>
#include <dilithium/engine/Engine.hpp>
#include <dilithium/gfx/Renderer.hpp>
#include <dilithium/platform/Window.hpp>
#include <dilithium/scenes/Scene.hpp>
#include <dilithium/scenes/SceneManager.hpp>

#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string_view>
#include <thread>
#include <utility>

namespace dilithium {
namespace {

/// How long to sleep when no frame was presented (minimized, or no area). The present paces every other frame; this
/// keeps an idle window from spinning a CPU core while still noticing a restore within a frame or so.
constexpr std::chrono::milliseconds kIdleSleep{16};

std::string_view appStateName(AppState state) {
    switch (state) {
    case AppState::Active:
        return "active";
    case AppState::Inactive:
        return "inactive";
    case AppState::Background:
        return "background";
    }
    DILITHIUM_UNREACHABLE("unknown AppState");
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

} // namespace

Engine::Impl::Impl(std::unique_ptr<Window> newWindow, std::unique_ptr<Renderer> newRenderer,
                   const CommandLine& commandLine)
    : options(parseRunOptions(commandLine.getAll())), window(std::move(newWindow)), renderer(std::move(newRenderer)),
      scenes(*renderer, *this) {
    if (!window || !renderer) {
        DILITHIUM_UNREACHABLE("Engine needs a window and a renderer");
    }
    if (options.frames) {
        logInfo("run: {} frames", *options.frames);
    }
}

FrameResult Engine::Impl::frame() {
    if (quitting) {
        return FrameResult::Finished;
    }
    ++frameNumber;
    if (appState.isFrozen(pausesWhenInactive)) {
        // The player is away: no update, no draw; the last frame stays on screen. The clock is reset on thaw.
        count(counts, FrameOutcome::Idle);
        std::this_thread::sleep_for(kIdleSleep);
        if (options.frames && frameNumber >= *options.frames) {
            return finishRun();
        }
        return FrameResult::Continue;
    }

    const float dt = clock.tick();

    scenes.performTransition();
    Scene& scene = scenes.getCurrent();
    scene.update(dt);
    scene.draw();

    const FrameOutcome outcome = renderer->drawFrame();
    count(counts, outcome);
    if (outcome == FrameOutcome::Idle) {
        std::this_thread::sleep_for(kIdleSleep);
    }

    if (options.frames && frameNumber >= *options.frames) {
        return finishRun();
    }
    return FrameResult::Continue;
}

void Engine::Impl::quitRequested() {
    switch (scenes.getCurrent().quitRequested()) {
    case QuitResponse::Quit:
        quit();
        return;
    case QuitResponse::Handled:
        DILITHIUM_LOG_DEBUG("quit requested; the scene took it over");
        return;
    }
    DILITHIUM_UNREACHABLE("unknown QuitResponse");
}

void Engine::Impl::windowFactsChanged(WindowFacts facts) {
    windowFacts = facts;
    if (const std::optional<AppState> changed = appState.update(facts)) {
        appStateChanged(*changed);
    }
}

void Engine::Impl::appStateSet(AppState state) {
    if (const std::optional<AppState> changed = appState.set(state)) {
        appStateChanged(*changed);
    }
}

/// Observers first (they save), then the scene on top (it may push a pause scene). A transition the scene asked for
/// is performed and drawn at once, since a frozen loop would not get to it; the thaw resets the clock so the first
/// frame back has a normal `dt` instead of the whole time away.
void Engine::Impl::appStateChanged(AppState state) {
    DILITHIUM_LOG_DEBUG("app state: {}", appStateName(state));
    for (const auto& observer : appStateObservers) {
        observer(state);
    }
    scenes.getCurrent().appStateChanged(state);
    if (scenes.performTransition()) {
        drawOnce();
    }
    if (!appState.isFrozen(pausesWhenInactive)) {
        clock.reset();
    }
}

/// One draw with no update, so a scene that just arrived (a pause menu) is on screen before the loop freezes.
void Engine::Impl::drawOnce() {
    scenes.getCurrent().draw();
    count(counts, renderer->drawFrame());
}

void Engine::Impl::resized() {
    const PixelSize pixels = window->getPixelSize();
    DILITHIUM_LOG_DEBUG("window is {}x{} pixels", pixels.width, pixels.height);
    scenes.getCurrent().resize();
}

/// The end of a `--frames` run: a summary line, and failure if the renderer's checks said anything, so a run cannot
/// pass by luck. (Validation errors already aborted; this catches warnings.)
FrameResult Engine::Impl::finishRun() const {
    logInfo("ran {} frames: {} presented, {} skipped, {} idle", frameNumber, counts.presented, counts.skipped,
            counts.idle);
    const uint32_t problems = renderer->getProblemsReported();
    if (problems > 0) {
        logError("the renderer reported {} problems during the run", problems);
        return FrameResult::Failed;
    }
    return FrameResult::Finished;
}

Engine::Engine(std::unique_ptr<Window> window, std::unique_ptr<Renderer> renderer, const CommandLine& commandLine)
    : impl(std::make_unique<Impl>(std::move(window), std::move(renderer), commandLine)) {
    logInfo("Dilithium2D {}", version());
}

Engine::~Engine() = default;

SceneManager& Engine::getScenes() {
    return impl->scenes;
}

void Engine::addAppStateObserver(std::function<void(AppState)> observer) {
    impl->appStateObservers.push_back(std::move(observer));
}

void Engine::setPausesWhenInactive(bool pauses) {
    impl->pausesWhenInactive = pauses;
}

} // namespace dilithium
