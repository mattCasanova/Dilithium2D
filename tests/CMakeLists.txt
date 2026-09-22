add_executable(dilithium_tests
    AssertTests.cpp
    ColorTests.cpp
    LogTests.cpp
    NonCopyableTests.cpp
    VersionTests.cpp)
target_link_libraries(dilithium_tests PRIVATE
    dilithium::dilithium
    Catch2::Catch2WithMain)
# Tests reach the engine's private headers too, so pure helpers can stay out of the public API.
target_include_directories(dilithium_tests PRIVATE ${PROJECT_SOURCE_DIR}/engine/src)
target_compile_definitions(dilithium_tests PRIVATE DILITHIUM_EXPECTED_VERSION="${PROJECT_VERSION}")

dilithium_set_warnings(dilithium_tests)
dilithium_enable_sanitizers(dilithium_tests)

include(${catch2_SOURCE_DIR}/extras/Catch.cmake)
catch_discover_tests(dilithium_tests)
