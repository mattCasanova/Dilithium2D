# AddressSanitizer + UndefinedBehaviorSanitizer for Dilithium2D's own targets, when DILITHIUM_SANITIZE is on.
# Any UBSan report aborts the program, so a run cannot pass with one in its output.
# Leak checking is not supported on Apple Silicon; it stays off.
function(dilithium_enable_sanitizers target)
    if(NOT DILITHIUM_SANITIZE)
        return()
    endif()
    if(MSVC)
        message(FATAL_ERROR "DILITHIUM_SANITIZE supports clang and gcc only")
    endif()

    set(flags -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer)
    target_compile_options(${target} PRIVATE ${flags})
    # PUBLIC: whatever links an instrumented library needs the sanitizer runtime too.
    target_link_options(${target} PUBLIC ${flags})
endfunction()
