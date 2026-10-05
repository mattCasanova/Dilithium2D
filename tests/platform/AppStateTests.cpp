#include <dilithium/platform/AppState.hpp>

#include <catch2/catch_test_macros.hpp>

using dilithium::AppState;
using dilithium::appStateName;

TEST_CASE("each app state has a name for the log", "[appstate]") {
    CHECK(appStateName(AppState::Active) == "active");
    CHECK(appStateName(AppState::Inactive) == "inactive");
    CHECK(appStateName(AppState::Background) == "background");
}
