// The one translation unit that compiles VMA's implementation. That code lives in VMA's header, which the build
// treats as a system header, so our warnings do not reach it.
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
