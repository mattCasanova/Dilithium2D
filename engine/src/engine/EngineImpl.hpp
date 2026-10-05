#pragma once

#include "engine/FrameClock.hpp"

#include <dilithium/engine/Application.hpp>
#include <dilithium/engine/Engine.hpp>
#include <dilithium/gfx/Renderer.hpp>
#include <dilithium/platform/AppState.hpp>
#include <dilithium/platform/Window.hpp>
#include <dilithium/scenes/SceneManager.hpp>
#include <dilithium/utilities/Assert.hpp>

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <vector>

namespace dilithium {

class CommandLine;

/// What one frame asks the platform loop to do next.
enum class FrameResult {
    Continue, ///< keep calling
    Finished, ///< a `--frames` run ended clean: exit 0
    Failed,   ///< a `--frames` run ended with validation messages: exit 1
};

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
};

/// Everything the engine owns, in creation order, so destruction runs the other way with no code for it: the scenes
/// first (they use the renderer), then the renderer (it draws into the window), then the window. Shared by
/// `Engine.cpp` (lifetime) and `main/SdlAdapter.cpp` (the SDL side: frames and events). Nothing here names SDL.
struct Engine::Impl final : Application {
    Impl(std::unique_ptr<Window> newWindow, std::unique_ptr<Renderer> newRenderer, const CommandLine& commandLine);

    /// `Application::quit`: the run ends at the start of the next frame.
    void quit() override { quitting = true; }
    [[nodiscard]] AppState getState() const override { return lastState; }

    /// Something about the window changed (focus, minimized, hidden): ask it which state that is.
    void windowStateChanged();
    /// A state was reported, by the window or by a phone's lifecycle: delivered to the game only when it differs
    /// from the last one delivered, then freeze or thaw.
    void appStateReported(AppState state);

    /// The player asked to quit (close button, Command-Q, the Dock): ask the scene on top; quit unless it took over.
    void quitRequested();

    /// One frame: the clock, a pending scene transition, update, draw, present.
    FrameResult frame();

    /// The window's pixel size changed: tell the current scene. The renderer finds out by itself each frame.
    void resized();

    std::optional<uint64_t> frameLimit; ///< `--frames N`: quit after N loop ticks, exit 1 if validation spoke
    std::unique_ptr<Window> window;
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
