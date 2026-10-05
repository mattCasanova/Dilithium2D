#pragma once

#include <string_view>

namespace dilithium {

/// The engine version, set by `project(VERSION ...)` in the top-level CMakeLists.txt.
std::string_view version();

} // namespace dilithium
