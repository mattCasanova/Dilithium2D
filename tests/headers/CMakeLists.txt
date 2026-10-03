# Every public header, compiled on its own with only the public include folder on the path. Proves two rules on
# every build, mechanically: each header under engine/include is self-contained (includes what it needs), and none
# reaches a third-party header (there is no SDL, Vulkan, VMA, glm or JSON include path here, so a leak fails to
# compile). The same rules a game that pulls the engine in through FetchContent relies on.
file(GLOB_RECURSE DILITHIUM_PUBLIC_HEADERS CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/engine/include/*.hpp)

set(check_sources)
foreach(header IN LISTS DILITHIUM_PUBLIC_HEADERS)
    file(RELATIVE_PATH include_path ${PROJECT_SOURCE_DIR}/engine/include ${header})
    string(MAKE_C_IDENTIFIER ${include_path} stem)
    set(source ${CMAKE_CURRENT_BINARY_DIR}/${stem}.cpp)
    file(WRITE ${source} "#include <${include_path}>\n")
    list(APPEND check_sources ${source})
endforeach()

add_library(dilithium_public_headers OBJECT ${check_sources})
target_include_directories(dilithium_public_headers PRIVATE ${PROJECT_SOURCE_DIR}/engine/include)
target_compile_features(dilithium_public_headers PRIVATE cxx_std_20)
target_compile_definitions(dilithium_public_headers PRIVATE $<$<CONFIG:Debug>:DILITHIUM_DEBUG=1>)
# Warnings and hardening, not clang-tidy: a file that only includes a header "uses" nothing, and the include checker
# would say so. The headers are linted through the sources that use them.
dilithium_set_warnings(dilithium_public_headers)
dilithium_enable_hardening(dilithium_public_headers)
