#include "engine/DefaultEngineImpl.hpp"
#include "engine/FrameClock.hpp"

#include <dilithium/engine/AppServices.hpp>
#include <dilithium/engine/DefaultEngine.hpp>
#include <dilithium/engine/Engine.hpp>
#include <dilithium/gfx/Renderer.hpp>
#include <dilithium/platform/AppState.hpp>
#include <dilithium/scenes/Scene.hpp>
#include <dilithium/scenes/SceneManager.hpp>
#include <dilithium/utilities/Assert.hpp>
#include <dilithium/utilities/CommandLine.hpp>
#include <dilithium/utilities/Log.hpp>
#include <dilithium/utilities/Version.hpp>

#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <stdexcept>
#include <thread>
#include <utility>

namespace dilithium {
namespace {

/// How long to sleep when no frame was presented (minimized, or no area). The present paces every other frame; this
/// keeps an idle window from spinning a CPU core while still noticing a restore within a frame or so.
constexpr std::chrono::milliseconds kIdleSleep{16};

} // namespace

void FrameCounts::add(FrameOutcome outcome) {
    switch (outcome) {
    case FrameOutcome::Presented:
        ++presented;
        return;
    case FrameOutcome::Skipped:
        ++skipped;
        return;
    case FrameOutcome::Idle:
        ++idle;
        return;
    }
    DILITHIUM_UNREACHABLE("unknown FrameOutcome");
}

DefaultEngine::Impl::Impl(std::unique_ptr<Renderer> newRenderer, AppServices& app, const CommandLine& commandLine)
    : frameLimit(commandLine.getCount("--frames")), renderer(std::move(newRenderer)), scenes(*renderer, app) {
    if (!renderer) {
        DILITHIUM_UNREACHABLE("Engine needs a renderer");
    }
    if (frameLimit) {
        if (*frameLimit == 0) {
            throw std::invalid_argument("--frames needs a count above zero");
        }
        logInfo("run: {} frames", *frameLimit);
    }
}

FrameResult DefaultEngine::Impl::frame() {
    if (quitting) {
        return FrameResult::Finished;
    }
    ++frameNumber;
    if (isFrozen()) {
        // The player is away: no update, no draw; the last frame stays on screen. The clock is reset on thaw.
        counts.add(FrameOutcome::Idle);
        std::this_thread::sleep_for(kIdleSleep);
        if (frameLimit && frameNumber >= *frameLimit) {
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
    counts.add(outcome);
    if (outcome == FrameOutcome::Idle) {
        std::this_thread::sleep_for(kIdleSleep);
    }

    if (frameLimit && frameNumber >= *frameLimit) {
        return finishRun();
    }
    return FrameResult::Continue;
}

void DefaultEngine::Impl::quitRequested() {
    switch (scenes.getCurrent().quitRequested()) {
    case QuitResponse::Quit:
        quitting = true;
        return;
    case QuitResponse::Handled:
        DILITHIUM_LOG_DEBUG("quit requested; the scene took it over");
        return;
    }
    DILITHIUM_UNREACHABLE("unknown QuitResponse");
}

void DefaultEngine::Impl::appStateReported(AppState state) {
    if (state == lastState) {
        return;
    }
    lastState = state;
    appStateChanged(state);
}

/// Observers first (the application, then anything a game added), then the scene on top (it may push a pause
/// scene). A transition the scene asked for is performed and drawn at once, since a frozen loop would not get to
/// it; the thaw resets the clock so the first frame back has a normal `dt` instead of the whole time away.
void DefaultEngine::Impl::appStateChanged(AppState state) {
    DILITHIUM_LOG_DEBUG("app state: {}", appStateName(state));
    for (const auto& observer : appStateObservers) {
        observer(state);
    }
    scenes.getCurrent().appStateChanged(state);
    if (scenes.performTransition()) {
        drawOnce();
    }
    if (!isFrozen()) {
        clock.reset();
    }
}

/// One draw with no update, so a scene that just arrived (a pause menu) is on screen before the loop freezes.
void DefaultEngine::Impl::drawOnce() {
    scenes.getCurrent().draw();
    counts.add(renderer->drawFrame());
}

/// The end of a `--frames` run: a summary line, and failure if the renderer's checks said anything, so a run cannot
/// pass by luck. (Validation errors already aborted; this catches warnings.)
FrameResult DefaultEngine::Impl::finishRun() const {
    logInfo("ran {} frames: {} presented, {} skipped, {} idle", frameNumber, counts.presented, counts.skipped,
            counts.idle);
    const uint32_t problems = renderer->getProblemsReported();
    if (problems > 0) {
        logError("the renderer reported {} problems during the run", problems);
        return FrameResult::Failed;
    }
    return FrameResult::Finished;
}

DefaultEngine::DefaultEngine(std::unique_ptr<Renderer> renderer, AppServices& app, const CommandLine& commandLine)
    : impl(std::make_unique<Impl>(std::move(renderer), app, commandLine)) {
    logInfo("Dilithium2D {}", version());
}

DefaultEngine::~DefaultEngine() = default;

SceneManager& DefaultEngine::getScenes() {
    return impl->scenes;
}

Renderer& DefaultEngine::getRenderer() {
    return *impl->renderer;
}

FrameResult DefaultEngine::frame() {
    return impl->frame();
}

void DefaultEngine::appStateReported(AppState state) {
    impl->appStateReported(state);
}

void DefaultEngine::resized() {
    impl->scenes.getCurrent().resize();
}

void DefaultEngine::quitRequested() {
    impl->quitRequested();
}

void DefaultEngine::quit() {
    impl->quitting = true;
}

AppState DefaultEngine::getState() const {
    return impl->lastState;
}

void DefaultEngine::addAppStateObserver(std::function<void(AppState)> observer) {
    impl->appStateObservers.push_back(std::move(observer));
}

void DefaultEngine::setPausesWhenInactive(bool pauses) {
    impl->pausesWhenInactive = pauses;
}

} // namespace dilithium
