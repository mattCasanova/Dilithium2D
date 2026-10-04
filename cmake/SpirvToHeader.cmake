# Turns one SPIR-V file into a header holding its words as a constexpr std::array<uint32_t, N>. Run by
# dilithium_add_shaders through `cmake -P`, with SPV, HEADER, NAMESPACE and NAME set on the command line.
#
# SPIR-V is stored little-endian; the words are assembled from the bytes here, so the array holds the right values
# on any host. The first word is the magic number 0x07230203, which the tests check.
foreach(variable SPV HEADER NAMESPACE NAME)
    if(NOT DEFINED ${variable})
        message(FATAL_ERROR "SpirvToHeader.cmake needs -D${variable}=...")
    endif()
endforeach()

file(READ ${SPV} hex HEX)
string(LENGTH "${hex}" hexLength)
math(EXPR byteCount "${hexLength} / 2")
math(EXPR remainder "${byteCount} % 4")
if(NOT remainder EQUAL 0)
    message(FATAL_ERROR "${SPV} is ${byteCount} bytes, not a whole number of SPIR-V words")
endif()
math(EXPR wordCount "${byteCount} / 4")

# Each 8 hex digits are one word's bytes b0 b1 b2 b3; little-endian, the value is 0xb3b2b1b0. Eight words per line
# keeps the file readable. (CMake's regex has no {n} repeat, hence the loop.)
string(REGEX MATCHALL "........" chunks "${hex}")
set(words "")
set(onLine 0)
foreach(chunk IN LISTS chunks)
    string(REGEX REPLACE "(..)(..)(..)(..)" "0x\\4\\3\\2\\1u" word "${chunk}")
    if(onLine EQUAL 8)
        string(APPEND words "\n    ")
        set(onLine 0)
    elseif(words)
        string(APPEND words " ")
    endif()
    string(APPEND words "${word},")
    math(EXPR onLine "${onLine} + 1")
endforeach()

file(WRITE ${HEADER} "// Generated from ${SPV} by SpirvToHeader.cmake. Do not edit; edit the GLSL.
#pragma once

#include <array>
#include <cstdint>

namespace ${NAMESPACE} {

inline constexpr std::array<uint32_t, ${wordCount}> ${NAME}{
    ${words}
};

} // namespace ${NAMESPACE}
")
