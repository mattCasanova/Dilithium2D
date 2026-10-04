# GLSL to SPIR-V at build time, embedded in the binary. The engine uses it for its own shaders; a game calls it the
# same way on its own.
#
#   dilithium_add_shaders(my_game NAMESPACE mygame::shaders FILES shaders/sprite.vert shaders/sprite.frag)
#
# Each file compiles with glslc (-Werror, Vulkan 1.3) into <build>/shaders/<name>.spv, and that into a header
# <build>/shaders/<name>_<stage>.hpp holding `inline constexpr std::array<uint32_t, N> k<Name><Stage>` in the
# namespace given. The target gets the headers' folder on its include path: `#include "shaders/sprite_vert.hpp"`
# gives `mygame::shaders::kSpriteVert`. A shader edit rebuilds only that header and what includes it. Nothing is
# read at run time, so there is no file to fail to find, on any platform.
#
# An engine shader a game never uses never reaches its binary: each lives in the object file that includes it,
# and the linker drops object files nothing references.
function(dilithium_add_shaders target)
    cmake_parse_arguments(SHADERS "" "NAMESPACE" "FILES" ${ARGN})
    if(NOT SHADERS_NAMESPACE OR NOT SHADERS_FILES)
        message(FATAL_ERROR "dilithium_add_shaders(<target> NAMESPACE <ns> FILES <glsl files...>)")
    endif()
    if(NOT Vulkan_GLSLC_EXECUTABLE)
        message(FATAL_ERROR "glslc not found (brew install shaderc); the Vulkan package reports no glslc component")
    endif()

    set(outputDir ${CMAKE_CURRENT_BINARY_DIR}/shaders)
    file(MAKE_DIRECTORY ${outputDir})

    foreach(source IN LISTS SHADERS_FILES)
        get_filename_component(absolute ${source} ABSOLUTE)
        get_filename_component(name ${source} NAME_WE)   # color
        get_filename_component(stage ${source} LAST_EXT) # .vert
        string(SUBSTRING ${stage} 1 -1 stage)             # vert
        set(spv ${outputDir}/${name}.${stage}.spv)
        set(header ${outputDir}/${name}_${stage}.hpp)

        # kColorVert: the file name and stage, each capitalized.
        string(SUBSTRING ${name} 0 1 nameFirst)
        string(SUBSTRING ${name} 1 -1 nameRest)
        string(SUBSTRING ${stage} 0 1 stageFirst)
        string(SUBSTRING ${stage} 1 -1 stageRest)
        string(TOUPPER ${nameFirst} nameFirst)
        string(TOUPPER ${stageFirst} stageFirst)
        set(identifier k${nameFirst}${nameRest}${stageFirst}${stageRest})

        add_custom_command(
            OUTPUT ${spv}
            COMMAND ${Vulkan_GLSLC_EXECUTABLE} -Werror --target-env=vulkan1.3 -o ${spv} ${absolute}
            DEPENDS ${absolute}
            COMMENT "glslc ${source}"
            VERBATIM)
        add_custom_command(
            OUTPUT ${header}
            COMMAND ${CMAKE_COMMAND}
                -DSPV=${spv} -DHEADER=${header} -DNAMESPACE=${SHADERS_NAMESPACE} -DNAME=${identifier}
                -P ${PROJECT_SOURCE_DIR}/cmake/SpirvToHeader.cmake
            DEPENDS ${spv} ${PROJECT_SOURCE_DIR}/cmake/SpirvToHeader.cmake
            COMMENT "embedding ${name}.${stage}.spv"
            VERBATIM)
        target_sources(${target} PRIVATE ${header})
    endforeach()

    target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
endfunction()
