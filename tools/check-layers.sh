#!/bin/sh
# Layering the compiler cannot check: which folders may include which third-party header. SDL belongs to platform/
# and shell/; Vulkan and VMA belong to gfx/. Run by ctest; prints each offending file.
set -eu
cd "$(dirname "$0")/.."

fail=0
check() { # $1 rule, $2 include pattern, $3 folders allowed to match it
    offenders=$(grep -rlE "$2" engine/ | grep -vE "$3" || true)
    if [ -n "$offenders" ]; then
        echo "$1:"
        echo "$offenders" | sed 's/^/  /'
        fail=1
    fi
}
check "SDL headers belong to platform/ and shell/ only" '^#include <SDL3/' '^engine/(src/platform|shell)/'
check "Vulkan and VMA headers belong to gfx/ only" '^#include <(vulkan/|vk_mem_alloc)' '^engine/src/gfx/'

[ "$fail" -eq 0 ] && echo "layers: every third-party include is where it belongs"
exit "$fail"
