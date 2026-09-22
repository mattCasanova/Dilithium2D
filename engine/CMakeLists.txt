add_library(dilithium STATIC
    src/core/Assert.cpp
    src/core/Color.cpp
    src/core/Log.cpp
    src/core/Version.cpp)
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
