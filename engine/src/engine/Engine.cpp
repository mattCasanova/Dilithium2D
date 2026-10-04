#include "engine/EngineImpl.hpp"
#include "engine/FrameTime.hpp"
#include "engine/RunOptions.hpp"
#include "engine/Torture.hpp"

#include <dilithium/core/Assert.hpp>
#include <dilithium/core/Log.hpp>
#include <dilithium/core/Version.hpp>
#include <dilithium/engine/CommandLine.hpp>
#include <dilithium/engine/Engine.hpp>
#include <dilithium/gfx/Renderer.hpp>
#include <dilithium/platform/Window.hpp>
#include <dilithium/scenes/Scene.hpp>
#include <dilithium/scenes/SceneManager.hpp>

#include <SDL3/SDL.h>

#include <cstdint>
#include <format>
#include <memory>
#include <utility>

namespace dilithium {
namespace {

/// How long to sleep when no frame was presented (minimized, or no area). The present paces every other frame; this
/// keeps an idle window from spinning a CPU core while still noticing a restore within a frame or so.
constexpr uint32_t kIdleSleepMs = 16;

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

} // namespace

Engine::Impl::Impl(std::unique_ptr<Window> newWindow, std::unique_ptr<Renderer> newRenderer,
                   const CommandLine& commandLine)
    : options(parseRunOptions(commandLine.all())), window(std::move(newWindow)), renderer(std::move(newRenderer)),
      scenes(*renderer, *this), lastFrameNs(SDL_GetTicksNS()) {
    if (!window || !renderer) {
        DILITHIUM_UNREACHABLE("Engine needs a window and a renderer");
    }
    if (options.frames || options.torture) {
        logInfo("run: {}{}", options.frames ? std::format("{} frames", *options.frames) : "until quit",
                options.torture ? ", with torture" : "");
    }
}

FrameResult Engine::Impl::frame() {
    if (quitting) {
        return FrameResult::Finished;
    }
    ++frameNumber;
    if (options.torture) {
        applyTorture(*window, tortureStep(frameNumber));
    }

    const uint64_t now = SDL_GetTicksNS();
    const float dt = frameSeconds(lastFrameNs, now);
    lastFrameNs = now;

    scenes.performTransition();
    Scene& scene = scenes.current();
    scene.update(dt);
    scene.draw();

    const FrameOutcome outcome = renderer->drawFrame();
    count(counts, outcome);
    if (outcome == FrameOutcome::Idle) {
        SDL_Delay(kIdleSleepMs);
    }

    if (options.frames && frameNumber >= *options.frames) {
        return finishRun();
    }
    return FrameResult::Continue;
}

void Engine::Impl::quitRequested() {
    switch (scenes.current().quitRequested()) {
    case QuitResponse::Quit:
        quit();
        return;
    case QuitResponse::Handled:
        DILITHIUM_LOG_DEBUG("quit requested; the scene took it over");
        return;
    }
    DILITHIUM_UNREACHABLE("unknown QuitResponse");
}

void Engine::Impl::resized() {
    const PixelSize pixels = window->pixelSize();
    DILITHIUM_LOG_DEBUG("window is {}x{} pixels", pixels.width, pixels.height);
    scenes.current().resize();
}

/// The end of a `--frames` run: a summary line, and failure if the renderer's checks said anything, so a run cannot
/// pass by luck. (Validation errors already aborted; this catches warnings.)
FrameResult Engine::Impl::finishRun() const {
    logInfo("ran {} frames: {} presented, {} skipped, {} idle", frameNumber, counts.presented, counts.skipped,
            counts.idle);
    const uint32_t problems = renderer->problemsReported();
    if (problems > 0) {
        logError("the renderer reported {} problems during the run", problems);
        return FrameResult::Failed;
    }
    return FrameResult::Finished;
}

Engine::Engine(std::unique_ptr<Window> window, std::unique_ptr<Renderer> renderer, const CommandLine& commandLine)
    : m_impl(std::make_unique<Impl>(std::move(window), std::move(renderer), commandLine)) {
    logInfo("Dilithium2D {}", version());
}

Engine::~Engine() = default;

SceneManager& Engine::scenes() {
    return m_impl->scenes;
}

} // namespace dilithium
