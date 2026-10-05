#pragma once

#include <source_location>
#include <string_view>

namespace dilithium::detail {

/// Logs the message with the caller's file, line and function, then aborts in debug builds or throws
/// `std::logic_error` in release. Call it through `DILITHIUM_UNREACHABLE`.
[[noreturn]] void unreachable(std::string_view message, std::source_location where = std::source_location::current());

/// The failure path of `DILITHIUM_ASSERT`: reports the condition's text and the message, then fails like `unreachable`.
[[noreturn]] void assertFailed(std::string_view condition, std::string_view message,
                               std::source_location where = std::source_location::current());

} // namespace dilithium::detail

/// Marks a branch that cannot happen: an unknown enum value, a state the code's own invariants rule out. Where
/// Swift would use `preconditionFailure`. Never a fallback that hides a bug. Active in every build.
#define DILITHIUM_UNREACHABLE(message) ::dilithium::detail::unreachable(message)

/// Checks an invariant in debug builds: if `condition` is false, logs it with the message and aborts (Mach 5's
/// `M5DEBUG_ASSERT`). Release builds still compile `condition` but never run it, so the check is free on per-frame
/// paths; never put a side effect in it. Expands to one expression, so it is safe as the body of a brace-less `if`.
/// A condition with a top-level comma (`std::is_same_v<A, B>`) needs its own parentheses.
#ifdef DILITHIUM_DEBUG
#define DILITHIUM_ASSERT(condition, message)                                                                           \
    (static_cast<bool>(condition) ? static_cast<void>(0) : ::dilithium::detail::assertFailed(#condition, message))
#else
#define DILITHIUM_ASSERT(condition, message) static_cast<void>(false && static_cast<bool>(condition))
#endif
