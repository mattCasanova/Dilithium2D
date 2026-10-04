#include "engine/AppStateTracker.hpp"

#include <dilithium/engine/Application.hpp>

#include <catch2/catch_test_macros.hpp>

using dilithium::AppState;
using dilithium::appStateFrom;
using dilithium::AppStateTracker;

TEST_CASE("the state follows the window facts: hidden or minimized wins, then focus", "[appstate]") {
    STATIC_CHECK(appStateFrom({.focused = true, .minimized = false, .hidden = false}) == AppState::Active);
    STATIC_CHECK(appStateFrom({.focused = false, .minimized = false, .hidden = false}) == AppState::Inactive);
    STATIC_CHECK(appStateFrom({.focused = true, .minimized = true, .hidden = false}) == AppState::Background);
    STATIC_CHECK(appStateFrom({.focused = true, .minimized = false, .hidden = true}) == AppState::Background);
    STATIC_CHECK(appStateFrom({.focused = false, .minimized = true, .hidden = true}) == AppState::Background);
}

TEST_CASE("a tracker starts active and reports a change once", "[appstate]") {
    AppStateTracker tracker;
    CHECK(tracker.state() == AppState::Active);
    CHECK_FALSE(tracker.update({.focused = true}).has_value()); // no change: not delivered
    CHECK(tracker.update({.focused = false}) == AppState::Inactive);
    CHECK_FALSE(tracker.update({.focused = false}).has_value());
    CHECK(tracker.update({.focused = false, .minimized = true}) == AppState::Background);
    CHECK(tracker.update({.focused = true}) == AppState::Active);
}

TEST_CASE("a phone sets the state directly", "[appstate]") {
    AppStateTracker tracker;
    CHECK(tracker.set(AppState::Background) == AppState::Background);
    CHECK_FALSE(tracker.set(AppState::Background).has_value());
    CHECK(tracker.state() == AppState::Background);
}

TEST_CASE("frozen in the background always, and while inactive only if asked", "[appstate]") {
    AppStateTracker tracker;
    CHECK_FALSE(tracker.frozen(true));
    CHECK_FALSE(tracker.frozen(false));

    REQUIRE(tracker.set(AppState::Inactive).has_value());
    CHECK(tracker.frozen(true));
    CHECK_FALSE(tracker.frozen(false));

    REQUIRE(tracker.set(AppState::Background).has_value());
    CHECK(tracker.frozen(true));
    CHECK(tracker.frozen(false));
}
