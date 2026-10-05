#include <dilithium/platform/AppState.hpp>
#include <dilithium/utilities/Assert.hpp>

#include <string_view>

namespace dilithium {

std::string_view appStateName(AppState state) {
    switch (state) {
    case AppState::Active:
        return "active";
    case AppState::Inactive:
        return "inactive";
    case AppState::Background:
        return "background";
    }
    DILITHIUM_UNREACHABLE("unknown AppState");
}

} // namespace dilithium
