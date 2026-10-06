#pragma once

namespace dilithium {

/// Base for a move-only class: copying is deleted, moving is allowed. A class that owns a raw handle still writes
/// its own move operations, so the moved-from object gives the handle up. For a class that must not move either,
/// see `NonMovable`.
class NonCopyable {
public:
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;

protected:
    NonCopyable() = default;
    ~NonCopyable() = default;

    NonCopyable(NonCopyable&&) = default;
    NonCopyable& operator=(NonCopyable&&) = default;
};

} // namespace dilithium
