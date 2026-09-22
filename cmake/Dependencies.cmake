# Third-party dependencies. Each fetch is a release tarball pinned by version and SHA-256 (a hash pins the
# exact bytes, which a tag alone does not); CLAUDE.md lists them. To bump one: change the URL, run
# `curl -fsSL <url> | shasum -a 256`, paste the new hash.
# SYSTEM marks fetched headers as system headers, so our -Werror never fires on them. Imported targets
# (Vulkan, a found SDL3) are system headers by default.
include(FetchContent)

find_package(Vulkan REQUIRED)

# SDL3: use an installed copy (Homebrew on the Mac), else build a static one from source (Linux, CI).
if(NOT TARGET SDL3::SDL3)
    find_package(SDL3 CONFIG QUIET)
endif()
if(NOT TARGET SDL3::SDL3)
    message(STATUS "SDL3 not installed; building it from source")
    set(SDL_SHARED OFF)
    set(SDL_STATIC ON)
    set(SDL_TEST_LIBRARY OFF)
    FetchContent_Declare(SDL3
        URL https://github.com/libsdl-org/SDL/releases/download/release-3.4.16/SDL3-3.4.16.tar.gz
        URL_HASH SHA256=7322236cd12090c3eb40b9728be4d49c76f66ad17d04369584d4ecad5cf77c68
        SYSTEM)
    FetchContent_MakeAvailable(SDL3)
endif()

FetchContent_Declare(VulkanMemoryAllocator
    URL https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/archive/refs/tags/v3.4.0.tar.gz
    URL_HASH SHA256=822aa850c6ce77346ae96a8a1d351d52e77e85929f35363849a0a4e638e0a2a1
    SYSTEM)

# Header-only; glm otherwise compiles itself into a library by default.
set(GLM_BUILD_LIBRARY OFF)
FetchContent_Declare(glm
    URL https://github.com/g-truc/glm/archive/refs/tags/1.0.3.tar.gz
    URL_HASH SHA256=6775e47231a446fd086d660ecc18bcd076531cfedd912fbd66e576b118607001
    SYSTEM)

FetchContent_Declare(nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
    URL_HASH SHA256=42f6e95cad6ec532fd372391373363b62a14af6d771056dbfc86160e6dfff7aa
    SYSTEM)

FetchContent_MakeAvailable(VulkanMemoryAllocator glm nlohmann_json)

if(DILITHIUM_BUILD_TESTS)
    FetchContent_Declare(Catch2
        URL https://github.com/catchorg/Catch2/archive/refs/tags/v3.16.0.tar.gz
        URL_HASH SHA256=0957cae5821b17ce07f0833aaa52b5137643a8382203221f363a8303c109af34
        SYSTEM)
    FetchContent_MakeAvailable(Catch2)
endif()
