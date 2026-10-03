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

    # `undefined` leaves out four groups worth having: a float divided by zero; an implicit conversion that changes
    # the value at run time (a size_t truncated to uint32_t through a cast -Wconversion cannot see); a null passed
    # where _Nonnull is declared; and out-of-bounds indexing of a local array. Not unsigned-integer-overflow: a hash
    # or a wrapping counter does that on purpose.
    set(flags
        -fsanitize=address,undefined,float-divide-by-zero,implicit-conversion,nullability,local-bounds
        -fno-sanitize-recover=all
        -fno-omit-frame-pointer)
    target_compile_options(${target} PRIVATE ${flags})
    # PUBLIC: whatever links an instrumented library needs the sanitizer runtime too.
    target_link_options(${target} PUBLIC ${flags})
endfunction()
