#pragma once

#include <dilithium/engine/Application.hpp>

#include <optional>

namespace dilithium {

/// The facts the window system reports, from which the state follows.
struct WindowFacts {
    bool focused = true;
    bool minimized = false;
    bool hidden = false;
};

/// Derives the state from the facts: hidden or minimized is `Background`; otherwise focus decides between `Active`
/// and `Inactive`. Pure, so it is tested as a table.
[[nodiscard]] constexpr AppState appStateFrom(WindowFacts facts) {
    if (facts.hidden || facts.minimized) {
        return AppState::Background;
    }
    return facts.focused ? AppState::Active : AppState::Inactive;
}

/// Keeps the current state and says when it changed, so a change is delivered once. The engine feeds it window
/// events on a desktop and the phone's own lifecycle events on a phone. Starts `Active`, as LiquidMetal2D does: a
/// window is usually not yet focused when the engine is built, and the focus event follows a moment later.
class AppStateTracker {
public:
    [[nodiscard]] AppState getState() const { return state; }

    /// A window fact changed. Returns the new state if it differs from the current one.
    [[nodiscard]] std::optional<AppState> update(WindowFacts facts);

    /// A phone said so directly. Returns the new state if it differs from the current one.
    [[nodiscard]] std::optional<AppState> set(AppState next);

    /// Whether the loop should stand still: always in the background, and while inactive when `pausesWhenInactive`.
    [[nodiscard]] bool isFrozen(bool pausesWhenInactive) const;

private:
    AppState state = AppState::Active;
};

} // namespace dilithium
