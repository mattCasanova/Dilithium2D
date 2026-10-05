#include "engine/AppStateTracker.hpp"

#include <dilithium/engine/Application.hpp>
#include <dilithium/utilities/Assert.hpp>

#include <optional>

namespace dilithium {

std::optional<AppState> AppStateTracker::update(WindowFacts facts) {
    return set(appStateFrom(facts));
}

std::optional<AppState> AppStateTracker::set(AppState next) {
    if (next == state) {
        return std::nullopt;
    }
    state = next;
    return state;
}

bool AppStateTracker::isFrozen(bool pausesWhenInactive) const {
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

} // namespace dilithium
