# Dilithium2D

C++20 + raw Vulkan 1.3 2D game engine. Mirrors the design of LiquidMetal2D (Swift/Metal, `~/src/games/LiquidMetal2D`): a 3D-capable renderer locked to 2D gameplay. It is a library: each game is its own repo and pulls the engine in by git tag through `FetchContent`. Demos live here, one executable each.

Plans live outside the repo, in `~/workspace/Dilithium2D/` (`roadmap.md`, one plan per demo rung).

## Layout

```
CMakeLists.txt  CMakePresets.json  .clang-format
cmake/          Dependencies.cmake  Warnings.cmake  Sanitizers.cmake
engine/
  include/dilithium/   public headers: everything a game may include. No SDL, Vulkan, VMA, glm or JSON types
    App.hpp            the one header every program starts from
    core/  gfx/        one folder per area, the same names in src/ and tests/
  src/                 private: core/, gfx/ (Vulkan), platform/ (SDL). Every .cpp here is the library (globbed)
  main/                SdlMain.cpp: the dilithium::main target, kept outside src/ so the library glob skips it
demos/d01_clear_screen/
tests/                 Catch2 unit tests, one file per unit, same area folders; every .cpp is a test (globbed)
shaders/               GLSL, compiled to SPIR-V at build time (from D2)
```

## Writing a program (demo or game)

`main.cpp` includes `<dilithium/App.hpp>`, subclasses `dilithium::App`, and defines `dilithium::createApp`. CMake links `dilithium::main` alone: it is SDL's entry point (the four `SDL_App*` callbacks) and brings the engine with it. No program file includes an SDL header; the engine owns SDL's main loop and calls the `App` once per frame.

## Rules

- **No globals, no singletons, no `static` mutable state.** Everything hangs off an object that is passed in.
- **Every dependency is `PRIVATE` and wrapped.** No public header includes a third-party header.
- **Plain, descriptive names** in code, log text and messages: `RenderCore`, `FramesInFlight`, `error:`. Themed names are for the engine's name and tagline only; a name must help someone debugging who has never heard the joke.
- Macros in public headers carry the `DILITHIUM_` prefix, since macros ignore namespaces and a game's own must not collide with ours. Private ones (`VK_CHECK`) need not.
- Vulkan through the C API (`vulkan/vulkan.h`), wrapped in small RAII classes that are neither copyable nor movable: `RenderCore` builds each in place. Members are declared in creation order, so they destruct in the right order.
- `VK_CHECK(expr)` checks a `VkResult`; `DILITHIUM_UNREACHABLE("…")` marks branches that cannot happen (debug: log + abort; release: throws `std::logic_error`). `DILITHIUM_ASSERT(condition, "…")` checks an invariant in debug only; release never runs the condition, so never put a side effect in it. No silent `default:` or fallback on unexpected state.
- Log with `logInfo` / `logWarning` / `logError` (`std::format` strings) from `<dilithium/core/Log.hpp>`, and `DILITHIUM_LOG_DEBUG(…)` for a line that exists in debug builds only. No `std::cout` in engine code.
- Tests may include private headers (`engine/src` is on their include path); keep pure helpers there, out of the public API.
- **Exceptions are on, in every build, for errors that end the program only:** `DILITHIUM_UNREACHABLE` and `VK_CHECK` in release, and start-up failures. Nothing in the frame loop throws on purpose. Each SDL callback catches at the boundary and logs, so a release build still says what went wrong. Table-based exceptions cost nothing on the normal path; the price is binary size (about 9% of code) and a slow throw.
- Return values or `std::optional`, no out-parameters in our own APIs.
- Namespace `dilithium`. `#pragma once`. `PascalCase` types, `camelCase` functions and variables, `m_` prefix on private members. 4-space indent, 120 columns; run `clang-format`.
- Strict warnings as errors and ASan + UBSan apply to our targets only, never to a game that links the engine.
- `.gitignore` drops any folder named `debug/`, `release/`, `bin/`, `obj/` or `log/`. Never name a source folder that.
- Sources and tests are globbed: adding a `.cpp` needs no CMake edit, but any `.cpp` under `engine/src/` or `tests/` gets built. Keep experiments elsewhere.

## Build & test

```bash
cmake --preset debug && cmake --build --preset debug && ctest --preset debug
./build/debug/demos/d01_clear_screen/d01_clear_screen                         # the window; quit with Cmd-Q
./build/debug/demos/d01_clear_screen/d01_clear_screen --frames 600 --torture  # D1's proof: exit 0, no validation output
```

- `debug`: Ninja, `build/debug`, ASan + UBSan (`DILITHIUM_SANITIZE=ON`), `DILITHIUM_DEBUG=1`. `release`: `build/release`, no sanitizers.
- `DILITHIUM_BUILD_DEMOS` / `DILITHIUM_BUILD_TESTS` default on only when this is the top-level project.
- In-source builds are refused.
- Every program understands two engine flags: `--frames N` quits after N loop ticks (exit 1 if validation reported anything), `--torture` resizes the window every 20 frames and minimizes it every 97 (restoring 10 later). Anything else on the command line is the game's.
- A Catch2 test name must not start with `-`: ctest passes the name on the command line, where Catch2 reads it as a flag.

**Validation:** debug builds run the Khronos validation layer with synchronization validation on (`validate_sync`, through `VK_EXT_layer_settings`), which checks that barriers and semaphores really order the GPU's work; core validation does not. A validation error logs `error: validation: …` and aborts at the call. `DILITHIUM_NO_VALIDATION=1` turns it off, loudly. Brew's layer manifest names its library by bare file name, so a program must have `/opt/homebrew/lib` in its search paths (CMake adds it to anything linking the engine); without it the layer fails to load, loudly. `VK_LOADER_DEBUG=layer` shows whether the loader inserted it.

**Claude Code sandbox:** configure, build and unit tests work inside it (the `xcrun_db … Operation not permitted` lines are harmless noise). Anything that creates a Vulkan instance (`vulkaninfo`, running a demo) needs Metal and must run outside it. So do `git init`, `commit` and `push`: the sandbox blocks writes to `.git/config` and `.git/hooks`. `$TMPDIR` is a different folder inside and outside the sandbox, so output captured outside it goes to a full path. To stop a running demo cleanly, `pkill -TERM -x d01_clear_screen` outside the sandbox (SDL turns SIGTERM into a quit). Not `-INT`: a job started with `&` from a non-interactive shell has SIGINT ignored, and SDL leaves it that way. Not `-f`: it also matches the shell that launched it.

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
