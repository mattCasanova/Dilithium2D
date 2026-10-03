# Everything a Dilithium2D target gets and a game that links the engine does not: strict warnings as errors,
# sanitizers (debug preset), clang-tidy (debug builds, top-level only), and library hardening.
include(${CMAKE_CURRENT_LIST_DIR}/Warnings.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/Sanitizers.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/Lint.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/Hardening.cmake)

function(dilithium_apply_guard_rails target)
    dilithium_set_warnings(${target})
    dilithium_enable_sanitizers(${target})
    dilithium_enable_lint(${target})
    dilithium_enable_hardening(${target})
endfunction()
