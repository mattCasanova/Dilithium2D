# Dilithium2D

C++20 + raw Vulkan 1.3 2D game engine. Mirrors the design of LiquidMetal2D (Swift/Metal, `~/src/games/LiquidMetal2D`): a 3D-capable renderer locked to 2D gameplay. It is a library: each game is its own repo and pulls the engine in by git tag through `FetchContent`. Demos live here, one executable each.

Plans live outside the repo, in `~/workspace/LM2D/VulkanEngine/` (`roadmap.md`, one plan per demo rung).

## Layout

```
CMakeLists.txt  CMakePresets.json  .clang-format
cmake/          Dependencies.cmake  Warnings.cmake  Sanitizers.cmake
engine/
  include/dilithium/   public headers: everything a game may include. No SDL, Vulkan, VMA, glm or JSON types
  src/                 private: core/, and later platform/ (SDL) and gfx/ (Vulkan)
demos/d01_clear_screen/
tests/                 Catch2 unit tests, one file per unit
shaders/               GLSL, compiled to SPIR-V at build time (from D2)
```

## Rules

- **No globals, no singletons, no `static` mutable state.** Everything hangs off an object that is passed in.
- **Every dependency is `PRIVATE` and wrapped.** No public header includes a third-party header.
- Vulkan through the C API (`vulkan/vulkan.h`), wrapped in small move-only RAII classes. Members are declared in creation order, so they destruct in the right order.
- `KHAAAN(expr)` checks a `VkResult`; `ILLOGICAL("…")` marks branches that cannot happen (debug: log + abort; release: throws `std::logic_error`). `LOGICAL(condition, "…")` checks an invariant in debug only; release never runs the condition, so never put a side effect in it. No silent `default:` or fallback on unexpected state.
- Log with `logInfo` / `logWarning` / `logError` (`std::format` strings) from `<dilithium/Log.hpp>`, and `CAPTAINS_LOG(…)` for a line that exists in debug builds only. No `std::cout` in engine code.
- Tests may include private headers (`engine/src` is on their include path); keep pure helpers there, out of the public API.
- Return values or `std::optional`, no out-parameters in our own APIs.
- Namespace `dilithium`. `#pragma once`. `PascalCase` types, `camelCase` functions and variables, `m_` prefix on private members. 4-space indent, 120 columns; run `clang-format`.
- Strict warnings as errors and ASan + UBSan apply to our targets only, never to a game that links the engine.
- `.gitignore` drops any folder named `debug/`, `release/`, `bin/`, `obj/` or `log/`. Never name a source folder that.

## Build & test

```bash
cmake --preset debug && cmake --build --preset debug && ctest --preset debug
```

- `debug`: Ninja, `build/debug`, ASan + UBSan (`DILITHIUM_SANITIZE=ON`), `DILITHIUM_DEBUG=1`. `release`: `build/release`, no sanitizers.
- `DILITHIUM_BUILD_DEMOS` / `DILITHIUM_BUILD_TESTS` default on only when this is the top-level project.
- In-source builds are refused.

**Claude Code sandbox:** configure, build and unit tests work inside it (the `xcrun_db … Operation not permitted` lines are harmless noise). Anything that creates a Vulkan instance (`vulkaninfo`, running a demo) needs Metal and must run outside it. So do `git init`, `commit` and `push`: the sandbox blocks writes to `.git/config` and `.git/hooks`.

## Dependencies

Pinned in `cmake/Dependencies.cmake` by release tarball + SHA-256. Never a branch.

| Library | Version | How |
|---|---|---|
| Vulkan loader + headers | system (brew 1.4.357) | `find_package(Vulkan)` |
| SDL3 | `release-3.4.16` | installed copy if found (brew 3.4.16), else fetched and built static |
| VulkanMemoryAllocator | `v3.4.0` | fetched, header-only |
| glm | `1.0.3` | fetched, header-only (`GLM_BUILD_LIBRARY OFF`) |
| nlohmann/json | `v3.12.0` | fetched, header-only |
| Catch2 | `v3.16.0` | fetched, tests only |
