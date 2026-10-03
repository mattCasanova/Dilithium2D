# clang-tidy on Dilithium2D's own targets, in Debug builds, when Dilithium2D is the top-level project. A game that
# pulls the engine in never runs it, and neither does fetched code. Findings fail the build (.clang-tidy says
# WarningsAsErrors: '*', so clang-tidy exits 1 on any finding, and CMake treats that as a failed compile).
#
# The version is pinned: whole check families are on, each LLVM release adds checks, and an unplanned upgrade
# would fail a clean build. Upgrading is a commit of its own: new pin, fix the new findings.
set(DILITHIUM_CLANG_TIDY_VERSION 22.1.8)

function(dilithium_find_clang_tidy)
    if(NOT PROJECT_IS_TOP_LEVEL OR NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        return()
    endif()

    find_program(DILITHIUM_CLANG_TIDY clang-tidy HINTS $ENV{HOME}/.local/bin)
    if(NOT DILITHIUM_CLANG_TIDY)
        message(FATAL_ERROR
            "clang-tidy not found. Debug builds lint; install the pinned version with: "
            "uv tool install clang-tidy==${DILITHIUM_CLANG_TIDY_VERSION}")
    endif()

    execute_process(COMMAND ${DILITHIUM_CLANG_TIDY} --version OUTPUT_VARIABLE version_text)
    if(NOT version_text MATCHES "LLVM version ${DILITHIUM_CLANG_TIDY_VERSION}")
        message(FATAL_ERROR
            "${DILITHIUM_CLANG_TIDY} is not clang-tidy ${DILITHIUM_CLANG_TIDY_VERSION}. Install the pinned version "
            "with: uv tool install clang-tidy==${DILITHIUM_CLANG_TIDY_VERSION}")
    endif()

    set(command ${DILITHIUM_CLANG_TIDY})
    if(APPLE)
        # CMake 4 passes no -isysroot on macOS; Apple's compiler finds the SDK alone, a standalone clang-tidy
        # does not and reports every standard header missing.
        execute_process(COMMAND xcrun --show-sdk-path OUTPUT_VARIABLE sdk OUTPUT_STRIP_TRAILING_WHITESPACE)
        list(APPEND command --extra-arg=-isysroot${sdk})
    endif()
    # clang-tidy is one LLVM release ahead of Apple's clang; its -Wextra adds a warning (unnamed fields in a
    # designated initializer, which every Vulkan struct here relies on) that the compiler never sees.
    list(APPEND command --extra-arg=-Wno-missing-designated-field-initializers)

    set(DILITHIUM_CLANG_TIDY_COMMAND ${command} PARENT_SCOPE)
    message(STATUS "clang-tidy ${DILITHIUM_CLANG_TIDY_VERSION}: ${DILITHIUM_CLANG_TIDY}")
endfunction()

function(dilithium_enable_lint target)
    if(DILITHIUM_CLANG_TIDY_COMMAND)
        set_target_properties(${target} PROPERTIES CXX_CLANG_TIDY "${DILITHIUM_CLANG_TIDY_COMMAND}")
    endif()
endfunction()
