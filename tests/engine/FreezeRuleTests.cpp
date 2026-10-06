#include "engine/DefaultEngineImpl.hpp"

#include <dilithium/platform/AppState.hpp>

#include <catch2/catch_test_macros.hpp>

using dilithium::AppState;
using dilithium::freezesLoop;

TEST_CASE("the loop freezes in the background always, and while inactive only if asked", "[engine]") {
    CHECK_FALSE(freezesLoop(AppState::Active, true));
    CHECK_FALSE(freezesLoop(AppState::Active, false));
    CHECK(freezesLoop(AppState::Inactive, true));
    CHECK_FALSE(freezesLoop(AppState::Inactive, false));
    CHECK(freezesLoop(AppState::Background, true));
    CHECK(freezesLoop(AppState::Background, false));
}
