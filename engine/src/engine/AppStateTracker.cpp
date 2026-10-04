#include "engine/AppStateTracker.hpp"

#include <dilithium/core/Assert.hpp>
#include <dilithium/engine/Application.hpp>

#include <optional>

namespace dilithium {

std::optional<AppState> AppStateTracker::update(WindowFacts facts) {
    return set(appStateFrom(facts));
}

std::optional<AppState> AppStateTracker::set(AppState state) {
    if (state == m_state) {
        return std::nullopt;
    }
    m_state = state;
    return state;
}

bool AppStateTracker::frozen(bool pausesWhenInactive) const {
    switch (m_state) {
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
