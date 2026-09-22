add_executable(dilithium_tests
    VersionTests.cpp)
target_link_libraries(dilithium_tests PRIVATE
    dilithium::dilithium
    Catch2::Catch2WithMain)
target_compile_definitions(dilithium_tests PRIVATE DILITHIUM_EXPECTED_VERSION="${PROJECT_VERSION}")

dilithium_set_warnings(dilithium_tests)
dilithium_enable_sanitizers(dilithium_tests)

include(${catch2_SOURCE_DIR}/extras/Catch.cmake)
catch_discover_tests(dilithium_tests)
