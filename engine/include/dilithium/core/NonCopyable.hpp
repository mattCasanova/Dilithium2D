#pragma once

namespace dilithium {

/// Base for classes that own a resource: copying is deleted, moving is allowed. A class that owns a raw handle
/// still writes its own move operations, so the moved-from object gives the handle up.
class NonCopyable {
protected:
    NonCopyable() = default;
    ~NonCopyable() = default;

    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
    NonCopyable(NonCopyable&&) = default;
    NonCopyable& operator=(NonCopyable&&) = default;
};

} // namespace dilithium
