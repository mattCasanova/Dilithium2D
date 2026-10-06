#pragma once

namespace dilithium {

/// Base for a class that stays where it is built: copying and moving are both deleted. The Vulkan wrappers, the
/// window, the engine and the interfaces scenes hold references to derive from it, so a class line says so and no
/// class repeats the four deleted members. An empty base: it costs nothing and cannot cause a diamond.
class NonMovable {
public:
    NonMovable(const NonMovable&) = delete;
    NonMovable& operator=(const NonMovable&) = delete;
    NonMovable(NonMovable&&) = delete;
    NonMovable& operator=(NonMovable&&) = delete;

protected:
    NonMovable() = default;
    ~NonMovable() = default;
};

} // namespace dilithium
