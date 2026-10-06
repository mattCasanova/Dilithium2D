# Every .cpp under src/ is the library, so adding a file needs no edit here. CONFIGURE_DEPENDS re-checks the folders
# on every build (milliseconds with Ninja). shell/ sits outside src/, so this cannot sweep it in; a platform-only
# folder (Android, Windows) gets its own glob behind a platform check when one arrives.
file(GLOB_RECURSE DILITHIUM_SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_library(dilithium STATIC ${DILITHIUM_SOURCES})
add_library(dilithium::dilithium ALIAS dilithium)

target_include_directories(dilithium
    PUBLIC include
    PRIVATE src)
target_compile_features(dilithium PUBLIC cxx_std_20)
target_compile_definitions(dilithium
    PUBLIC $<$<CONFIG:Debug>:DILITHIUM_DEBUG=1>
    PRIVATE DILITHIUM_VERSION="${PROJECT_VERSION}")

# PRIVATE, all but one: no public header includes a third-party header. SDL sits behind Window and the SDL main
# shell, Vulkan and VMA behind RenderCore, JSON behind one loader. glm is PUBLIC on purpose: the public math types
# are its types (see include/dilithium/math/Types.hpp).
target_link_libraries(dilithium
    PUBLIC glm::glm
    PRIVATE
    Vulkan::Vulkan
    SDL3::SDL3
    GPUOpen::VulkanMemoryAllocator
    nlohmann_json::nlohmann_json)

dilithium_apply_guard_rails(dilithium)

# The engine's own shaders, compiled to SPIR-V and embedded (see cmake/Shaders.cmake). From D2.
dilithium_add_shaders(dilithium NAMESPACE dilithium::shaders FILES
    ${PROJECT_SOURCE_DIR}/shaders/color.vert
    ${PROJECT_SOURCE_DIR}/shaders/color.frag)

# The entry point: main() and the Shell in shell/Shell.cpp, which hands the platform four callbacks that build the game's
# Application and forward everything to it. An OBJECT library, so its object files are always linked in, and a
# target of its own, so the unit tests (Catch2's main) and tools with their own main() can link the engine without
# it. A game links dilithium::main alone: it brings the engine with it (listing both puts libdilithium.a on the link
# line twice, and Apple's linker warns).
add_library(dilithium_main OBJECT shell/Shell.cpp)
add_library(dilithium::main ALIAS dilithium_main)

target_link_libraries(dilithium_main
    PUBLIC dilithium
    PRIVATE SDL3::SDL3)

dilithium_apply_guard_rails(dilithium_main)
