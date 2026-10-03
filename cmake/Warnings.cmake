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
            -Wcast-qual
            -Wdocumentation
            -Wsuggest-override
            -Wsuggest-destructor-override
            -Wzero-as-null-pointer-constant
            -Wunreachable-code
            -Wunused-macros)
        # Not -Weverything: clang's manual advises against it (some of its warnings contradict each other, and every
        # release adds more). Not -Wswitch-enum: it would make every switch over a Vulkan enum list hundreds of
        # values; -Wswitch plus the no-silent-default rule does the job.
    endif()
    set_target_properties(${target} PROPERTIES COMPILE_WARNING_AS_ERROR ON)
endfunction()
