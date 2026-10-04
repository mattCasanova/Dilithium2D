cmake_minimum_required(VERSION 3.25)

# Before project(), so an in-source configure stops before it writes build files into the tree.
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
    message(FATAL_ERROR
        "In-source builds are not allowed. Delete CMakeCache.txt and CMakeFiles/, then use a preset: "
        "cmake --preset debug")
endif()

project(Dilithium2D
    VERSION 0.1.0
    DESCRIPTION "A C++20 + Vulkan 2D game engine"
    LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Demos and tests build only when Dilithium2D is the top-level project, not when a game pulls it in.
option(DILITHIUM_BUILD_DEMOS "Build the demo programs" ${PROJECT_IS_TOP_LEVEL})
option(DILITHIUM_BUILD_TESTS "Build the unit tests" ${PROJECT_IS_TOP_LEVEL})
option(DILITHIUM_SANITIZE "Build Dilithium2D's own targets with ASan + UBSan" OFF)

include(${PROJECT_SOURCE_DIR}/cmake/GuardRails.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/Shaders.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/Dependencies.cmake)

dilithium_find_clang_tidy()

add_subdirectory(engine)

if(DILITHIUM_BUILD_DEMOS)
    add_subdirectory(demos)
endif()

if(DILITHIUM_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
