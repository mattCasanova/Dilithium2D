#pragma once

#include "engine/FrameClock.hpp"

#include <dilithium/engine/DefaultEngine.hpp>
#include <dilithium/engine/Engine.hpp>
#include <dilithium/gfx/Renderer.hpp>
#include <dilithium/platform/AppState.hpp>
#include <dilithium/scenes/SceneManager.hpp>
#include <dilithium/utilities/Assert.hpp>

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <vector>

namespace dilithium {

class AppServices;
class CommandLine;

/// Whether the loop stands still in `state`: always in the background, and while inactive when `pausesWhenInactive`.
inline bool freezesLoop(AppState state, bool pausesWhenInactive) {
    switch (state) {
    case AppState::Active:
        return false;
    case AppState::Inactive:
        return pausesWhenInactive;
    case AppState::Background:
        return true;
    }
    DILITHIUM_UNREACHABLE("unknown AppState");
}

/// How a run's frames went, for the summary a `--frames` run ends with.
struct FrameCounts {
    uint64_t presented = 0;
    uint64_t skipped = 0;
    uint64_t idle = 0;

    void add(FrameOutcome outcome);
};

/// Everything the engine owns, in creation order, so destruction runs the other way with no code for it: the scenes
/// first (they use the renderer), then the renderer.
struct DefaultEngine::Impl {
    Impl(std::unique_ptr<Renderer> newRenderer, AppServices& app, const CommandLine& commandLine);

    FrameResult frame();
    void appStateReported(AppState state);
    void quitRequested();

    std::optional<uint64_t> frameLimit; ///< `--frames N`: quit after N loop ticks, exit 1 if validation spoke
    std::unique_ptr<Renderer> renderer;
    SceneManager scenes;
    uint64_t frameNumber = 0;              ///< loop ticks so far, presented or not
    bool quitting = false;                 ///< `quit()` was called; the next frame ends the run
    AppState lastState = AppState::Active; ///< what the game was last told; a window is rarely focused yet when the
                                           ///< engine is built, and the focus event follows a moment later
    bool pausesWhenInactive = true;
    std::vector<std::function<void(AppState)>> appStateObservers;
    FrameCounts counts;
    FrameClock clock;

private:
    /// Whether the loop stands still right now. A `--frames` run never pauses for lack of focus: it is automated,
    /// nobody is at the keyboard, and a window launched from a script may never be focused at all. Minimized still
    /// freezes it.
    [[nodiscard]] bool isFrozen() const { return freezesLoop(lastState, pausesWhenInactive && !frameLimit); }
    void appStateChanged(AppState state);
    void drawOnce();
    [[nodiscard]] FrameResult finishRun() const;
};

} // namespace dilithium
