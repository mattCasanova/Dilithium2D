# Strict warnings, as errors, for Dilithium2D's own targets. Third-party code keeps its own flags,
# and nothing here reaches a game that links the engine.
# One-off override while experimenting: cmake --preset debug --compile-no-warning-as-error
function(dilithium_set_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${target} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wshadow
            -Wconversion
            -Wsign-conversion
            -Wnon-virtual-dtor
            -Wold-style-cast
            -Wdouble-promotion
            -Wimplicit-fallthrough
            -Woverloaded-virtual
            -Wextra-semi
            -Wnull-dereference
            -Wformat=2
            -Wundef
            -Wcast-qual)
    endif()
    set_target_properties(${target} PROPERTIES COMPILE_WARNING_AS_ERROR ON)
endfunction()
