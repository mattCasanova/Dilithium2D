#include <dilithium/core/Version.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("version matches the CMake project version", "[version]") {
    CHECK(dilithium::version() == DILITHIUM_EXPECTED_VERSION);
}
