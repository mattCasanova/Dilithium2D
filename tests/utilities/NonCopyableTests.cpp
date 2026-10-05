#include <dilithium/utilities/NonCopyable.hpp>

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

namespace {

class Owner : dilithium::NonCopyable {};

} // namespace

TEST_CASE("NonCopyable makes a class move-only", "[noncopyable]") {
    STATIC_CHECK_FALSE(std::is_copy_constructible_v<Owner>);
    STATIC_CHECK_FALSE(std::is_copy_assignable_v<Owner>);
    STATIC_CHECK(std::is_nothrow_move_constructible_v<Owner>);
    STATIC_CHECK(std::is_nothrow_move_assignable_v<Owner>);
}
