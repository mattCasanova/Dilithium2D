#!/bin/sh
# Regenerates docs/architecture/generated/ from the code with clang-uml (brew install clang-uml), reading the debug
# build's compile commands. Each diagram is written as Mermaid text and wrapped in a Markdown page, which GitHub and
# VS Code (markdown-mermaid extension) render. Run after an API change and commit the result.
set -eu

cd "$(git rev-parse --show-toplevel)"
command -v clang-uml >/dev/null || { echo "clang-uml not found: brew install clang-uml" >&2; exit 1; }
[ -f build/debug/compile_commands.json ] || { echo "no debug build: cmake --preset debug" >&2; exit 1; }

# clang-uml's own clang needs telling where the macOS SDK and its builtin headers are, as the standalone clang-tidy
# does; Apple's compiler finds both by itself.
sdk=$(xcrun --show-sdk-path)
resources=$(ls -d "$(brew --prefix llvm@22)"/lib/clang/*/ | head -1)

clang-uml -g mermaid --add-compile-flag="-isysroot$sdk" --add-compile-flag="-resource-dir=$resources" "$@"

for mmd in docs/architecture/generated/*.mmd; do
    md="${mmd%.mmd}.md"
    title=$(sed -n 's/^title: //p' "$mmd" | head -1)
    {
        echo "# $title"
        echo
        echo "Generated from the code by \`tools/diagrams.sh\` (clang-uml). Do not edit; regenerate."
        echo
        echo '```mermaid'
        # The front matter (--- title ---) is Mermaid's own and renders as the diagram's title.
        cat "$mmd"
        echo '```'
    } > "$md"
done
echo "diagrams: $(ls docs/architecture/generated/*.md | wc -l | tr -d ' ') pages under docs/architecture/generated/"
