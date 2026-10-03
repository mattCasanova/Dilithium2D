# Every .cpp under tests/ is a test file, so adding one needs no edit here.
file(GLOB_RECURSE DILITHIUM_TEST_SOURCES CONFIGURE_DEPENDS *.cpp)
add_executable(dilithium_tests ${DILITHIUM_TEST_SOURCES})
# Vulkan::Headers, not Vulkan::Vulkan: the choice tests use Vulkan's structs but never call Vulkan or need a GPU.
target_link_libraries(dilithium_tests PRIVATE
    dilithium::dilithium
    Catch2::Catch2WithMain
    Vulkan::Headers)
# Tests reach the engine's private headers too, so pure helpers can stay out of the public API.
target_include_directories(dilithium_tests PRIVATE ${PROJECT_SOURCE_DIR}/engine/src)
target_compile_definitions(dilithium_tests PRIVATE DILITHIUM_EXPECTED_VERSION="${PROJECT_VERSION}")

dilithium_apply_guard_rails(dilithium_tests)

include(${catch2_SOURCE_DIR}/extras/Catch.cmake)
catch_discover_tests(dilithium_tests)

# Every public header compiles alone, with no third-party include path: see headers/CMakeLists.txt.
add_subdirectory(headers)
