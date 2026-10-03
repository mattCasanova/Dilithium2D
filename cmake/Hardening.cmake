# Run-time checks inside the standard library and the compiler's stack protection, for Dilithium2D's own targets.
#
# libc++ hardening (LLVM 18+, Apple's libc++ included) adds checks to the library itself: operator[] bounds, iterator
# validity, null dereference of smart pointers. Debug builds take the DEBUG mode (every check); release builds the
# FAST mode (the cheap ones, kept in shipped builds). This is why `v[i]` needs no `.at()`: the subscript is checked.
# The modes are ABI-compatible with each other and with an unhardened libc++, so fetched code and a game that
# links the engine are unaffected.
function(dilithium_enable_hardening target)
    if(MSVC)
        # MSVC's STL checks bounds in debug by default (_CONTAINER_DEBUG_LEVEL) and /GS is on; nothing to add yet.
        return()
    endif()
    target_compile_definitions(${target} PRIVATE
        $<IF:$<CONFIG:Debug>,_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_DEBUG,_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_FAST>)
    # Stack canaries on every function with a buffer, and checked forms of the C string and memory functions
    # (_FORTIFY_SOURCE needs optimization, so release only).
    target_compile_options(${target} PRIVATE -fstack-protector-strong)
    target_compile_definitions(${target} PRIVATE $<$<NOT:$<CONFIG:Debug>>:_FORTIFY_SOURCE=3>)
endfunction()
