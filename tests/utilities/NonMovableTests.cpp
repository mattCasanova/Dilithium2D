#include <dilithium/utilities/NonMovable.hpp>

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

namespace {

class Pinned : dilithium::NonMovable {
public:
    Pinned() = default;
};

} // namespace

TEST_CASE("a NonMovable class can be built but neither copied nor moved", "[nonmovable]") {
    STATIC_CHECK(std::is_default_constructible_v<Pinned>);
    STATIC_CHECK_FALSE(std::is_copy_constructible_v<Pinned>);
    STATIC_CHECK_FALSE(std::is_copy_assignable_v<Pinned>);
    STATIC_CHECK_FALSE(std::is_move_constructible_v<Pinned>);
    STATIC_CHECK_FALSE(std::is_move_assignable_v<Pinned>);
    STATIC_CHECK(sizeof(Pinned) == 1); // an empty base costs nothing
}
