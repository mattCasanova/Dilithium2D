#pragma once

#include "engine/RunOptions.hpp"

#include <dilithium/engine/Engine.hpp>
#include <dilithium/gfx/Renderer.hpp>
#include <dilithium/platform/Window.hpp>
#include <dilithium/scenes/SceneManager.hpp>

#include <cstdint>
#include <memory>

namespace dilithium {

class CommandLine;

/// What one frame asks the platform loop to do next.
enum class FrameResult {
    Continue, ///< keep calling
    Finished, ///< a `--frames` run ended clean: exit 0
    Failed,   ///< a `--frames` run ended with validation messages: exit 1
};

/// How a run's frames went, for the summary a `--frames` run ends with.
struct FrameCounts {
    uint64_t presented = 0;
    uint64_t skipped = 0;
    uint64_t idle = 0;
};

/// Everything the engine owns, in creation order, so destruction runs the other way with no code for it: the scenes
/// first (they use the renderer), then the renderer (it draws into the window), then the window. Shared by
/// `Engine.cpp` (lifetime) and `EngineLoop.cpp` (frames and events).
struct Engine::Impl {
    Impl(std::unique_ptr<Window> newWindow, std::unique_ptr<Renderer> newRenderer, const CommandLine& commandLine);

    /// One frame: the torture step, the clock, a pending scene transition, update, draw, present.
    FrameResult frame();

    /// The window's pixel size changed: tell the current scene. The renderer finds out by itself each frame.
    void resized();

    RunOptions options;
    std::unique_ptr<Window> window;
    std::unique_ptr<Renderer> renderer;
    SceneManager scenes;
    uint64_t frameNumber = 0; ///< loop ticks so far, presented or not
    FrameCounts counts;
    uint64_t lastFrameNs;

private:
    [[nodiscard]] FrameResult finishRun() const;
};

} // namespace dilithium
