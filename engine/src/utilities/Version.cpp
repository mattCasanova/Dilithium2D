#include <dilithium/utilities/Version.hpp>

#include <string_view>

namespace dilithium {

std::string_view version() {
    return DILITHIUM_VERSION;
}

} // namespace dilithium
