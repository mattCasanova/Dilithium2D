add_library(dilithium STATIC
    src/core/Assert.cpp
    src/core/Color.cpp
    src/core/FrameTime.cpp
    src/core/Log.cpp
    src/core/Version.cpp
    src/gfx/DeviceChoices.cpp
    src/gfx/SwapchainChoices.cpp
    src/platform/Window.cpp)
add_library(dilithium::dilithium ALIAS dilithium)

target_include_directories(dilithium
    PUBLIC include
    PRIVATE src)
target_compile_features(dilithium PUBLIC cxx_std_20)
target_compile_definitions(dilithium
    PUBLIC $<$<CONFIG:Debug>:DILITHIUM_DEBUG=1>
    PRIVATE DILITHIUM_VERSION="${PROJECT_VERSION}")

# All PRIVATE: no public header includes a third-party header. SDL sits behind Window and the SDL main
# shell, Vulkan and VMA behind WarpCore, JSON behind one loader.
target_link_libraries(dilithium PRIVATE
    Vulkan::Vulkan
    SDL3::SDL3
    GPUOpen::VulkanMemoryAllocator
    glm::glm
    nlohmann_json::nlohmann_json)

dilithium_set_warnings(dilithium)
dilithium_enable_sanitizers(dilithium)

# The entry point: SDL's main() drives the game's App through the four SDL_App* callbacks in SdlMain.cpp. An
# OBJECT library, so its object file is always linked in, and a target of its own, so the unit tests (Catch2's
# main) and tools with their own main() can link the engine without it. A game links dilithium::main alone: it
# brings the engine with it (listing both puts libdilithium.a on the link line twice, and Apple's linker warns).
add_library(dilithium_main OBJECT src/platform/SdlMain.cpp)
add_library(dilithium::main ALIAS dilithium_main)

target_include_directories(dilithium_main PRIVATE src)
target_link_libraries(dilithium_main
    PUBLIC dilithium
    PRIVATE SDL3::SDL3)

dilithium_set_warnings(dilithium_main)
dilithium_enable_sanitizers(dilithium_main)
