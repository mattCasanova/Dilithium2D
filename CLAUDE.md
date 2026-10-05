# Dilithium2D

C++20 + raw Vulkan 1.3 2D game engine. Mirrors the design of LiquidMetal2D (Swift/Metal, `~/src/games/LiquidMetal2D`): a 3D-capable renderer locked to 2D gameplay. It is a library: each game is its own repo and pulls the engine in by git tag through `FetchContent`. Demos live here, one executable each.

Plans live outside the repo, in `~/workspace/Dilithium2D/` (`roadmap.md`, one plan per demo rung).

## Layout

```
CMakeLists.txt  CMakePresets.json  .clang-format  .clang-tidy (tests/.clang-tidy drops the magic-number rule)
cmake/          Dependencies.cmake  GuardRails.cmake (Warnings, Sanitizers, Lint, Hardening: one call per target)
                Shaders.cmake (dilithium_add_shaders: GLSL to SPIR-V to an embedded header)  SpirvToHeader.cmake
engine/
  include/dilithium/   public headers: everything a game may include. No SDL, Vulkan, VMA, glm or JSON types
    engine/            Engine, Application: what Engine::create builds, and what a scene may ask of the program
    scenes/            Scene, SceneManager, SceneServices: the game's code and how it changes screens
    gfx/               Renderer (the interface), Color; renderers/DefaultRenderer (the engine's own, pimpl)
    platform/          Window (pimpl: the window system is in the .cpp)
    utilities/  math/  Assert, Log, NonCopyable, Version, CommandLine; Math, Types (glm aliases), Bezier, Shapes, Intersect, Easing
  src/                 private, the same tree: engine/ (EngineImpl, AppStateTracker, FrameClock),
                       scenes/, platform/ (SDL: Window, and WindowSurface, where the window meets Vulkan),
                       gfx/renderers/ (RenderCore), gfx/vulkan/, gfx/swapchain/, gfx/shaders/, gfx/buffers/.
                       Every .cpp here is the library (globbed)
  main/                the dilithium::main target: SdlMain.cpp (the four SDL_App* callbacks) and SdlAdapter (SDL's
                       events and answers mapped to the engine's). Kept outside src/ so the library glob skips it
demos/                 one executable each: d01_clear_screen, d02_triangle
docs/architecture/      README.md: the hand-drawn layer diagram; generated/: class, package, sequence and include
                       diagrams drawn from the code by tools/diagrams.sh (clang-uml), as Mermaid in Markdown
tests/                 Catch2 unit tests, one file per unit, same area folders; every *Tests.cpp is a test (globbed)
  stress/              stress_run: a program that resizes and minimizes the window while drawing; the engine's proof
  headers/             compiles each public header alone with no third-party include path: a leak fails the build
shaders/               GLSL (#version 450); compiled by glslc at build time and embedded as `dilithium::shaders::k<Name><Stage>`
                       (see cmake/Shaders.cmake); a game adds its own with the same CMake call
```

## Writing a program (demo or game)

A game is scenes plus one function. Each scene subclasses `dilithium::Scene` (constructor and destructor are its setup and shutdown; `update(dt)`, `draw()`, and the optional `resize()` and `resume()`) and takes `SceneServices&` in its constructor: `renderer` to draw with, `scenes` to ask for transitions (`set`, `push`, `pop`, performed at the start of the next frame). The function is `dilithium::Engine::create(const CommandLine&)`: build a `Window`, build a `DefaultRenderer` on it (or any other `Renderer`), build the `Engine` from both, register the scenes with `engine->getScenes().add<MyScene>(MyId::Menu)`, `start` the first, return the engine. `demos/d01_clear_screen/main.cpp` is the whole pattern in 50 lines. **Quitting:** the engine never quits on its own. The close button, Command-Q, the Dock's Quit and a logout all become `Scene::quitRequested()` on the scene on top; the default answer quits at once, and a scene returns `QuitResponse::Handled` to take over (prompt, save, then `services.app.quit()`). Ctrl-C and SIGTERM in a terminal quit at once, before the game hears anything: a developer's deliberate quit. **App state:** `AppState` is `Active`, `Inactive` (visible, no focus: Command-Tab, another window on top) or `Background` (minimized, hidden, or a phone's home screen). On a change the engine tells `Engine::addAppStateObserver` callbacks first, then the scene on top through `Scene::appStateChanged`; if the scene pushed a pause scene, that transition runs and is drawn once; then the loop freezes (no update, no draw) while the player is away: always in the background, and while inactive unless `engine->setPausesWhenInactive(false)`. On return the clock resets, so the first frame back has a normal `dt`. Nothing pops on return: the player dismisses the pause menu. `services.app.getState()` answers any time. The engine never shows UI of its own. CMake links `dilithium::main` alone: it is SDL's entry point (the four `SDL_App*` callbacks) and brings the engine with it. No program file includes an SDL or Vulkan header; the engine owns the main loop and runs one frame per callback.

## Rules

- **No globals, no singletons, no `static` mutable state.** Everything hangs off an object that is passed in. The one exception is the atomic flag a signal handler sets in `SdlMain.cpp`: a handler can reach nothing else.
- **SDL lives in `platform/` and `main/`; Vulkan and VMA live in `gfx/`.** Graphics never names the window system: it asks the window for the instance extensions it needs and for its surface through `platform/WindowSurface.hpp`, the one place both meet. That header includes SDL's `SDL_vulkan.h`, which defines the two Vulkan handle types itself, so `platform/` includes no Vulkan header. `tools/check-layers.sh` greps for both rules on every `ctest`.
- **Every dependency is `PRIVATE` and wrapped, but glm.** No public header includes a third-party header, except that the public math types are glm's (`<dilithium/math/Types.hpp>`: `Vec2`, `Vec3`, `Vec4`, `Mat4`), so glm is `PUBLIC`, built with intrinsics and Vulkan's depth range for engine and game alike. Its aligned types cannot be built in a constant expression, so functions over them are `inline`, not `constexpr`. The header check (`tests/headers/`) allows glm and nothing else.
- **Plain, descriptive names** in code, log text and messages: `RenderCore`, `FramesInFlight`, `error:`. Themed names are for the engine's name and tagline only; a name must help someone debugging who has never heard the joke.
- Macros in public headers carry the `DILITHIUM_` prefix, since macros ignore namespaces and a game's own must not collide with ours. Private ones (`VK_CHECK`) need not.
- Vulkan through the C API (`vulkan/vulkan.h`), wrapped in small RAII classes that are neither copyable nor movable: `RenderCore` builds each in place. Members are declared in creation order, so they destruct in the right order.
- `VK_CHECK(expr)` checks a `VkResult`; `DILITHIUM_UNREACHABLE("…")` marks branches that cannot happen (debug: log + abort; release: throws `std::logic_error`). `DILITHIUM_ASSERT(condition, "…")` checks an invariant in debug only; release never runs the condition, so never put a side effect in it. No silent `default:` or fallback on unexpected state.
- Log with `logInfo` / `logWarning` / `logError` (`std::format` strings) from `<dilithium/utilities/Log.hpp>`, and `DILITHIUM_LOG_DEBUG(…)` for a line that exists in debug builds only. No `std::cout` in engine code.
- Tests may include private headers (`engine/src` is on their include path); keep pure helpers there, out of the public API.
- **Exceptions are on, in every build, for errors that end the program only:** `DILITHIUM_UNREACHABLE` and `VK_CHECK` in release, and start-up failures. Nothing in the frame loop throws on purpose. Each SDL callback catches at the boundary and logs, so a release build still says what went wrong. Table-based exceptions cost nothing on the normal path; the price is binary size (about 9% of code) and a slow throw.
- Return values or `std::optional`, no out-parameters in our own APIs.
- Namespace `dilithium`. `#pragma once`. `PascalCase` types, `camelCase` functions, variables and members (no `m_`: it makes the code ugly and everything talks through interfaces anyway), `k` prefix on constants. Accessors start with `get` (`getFormat()`, `getScenes()`), since a field and a method cannot share a name; `handle()` on the Vulkan wrappers is the one exception, and predicates read as `isMinimized()`, `has(flag)`. 4-space indent, 120 columns; run `clang-format`.
- **No magic numbers.** A number whose meaning is not obvious where it is used gets a name; plain arithmetic (`x / 2`, `count - 1`) needs none. Tests are exempt: an expected value reads best inline.
- **A constant lives with the code that owns it.** One `.cpp`: `constexpr` in its anonymous namespace. One class: a `static constexpr` member. Several files: `inline constexpr` in the owning area's header. No project-wide constants file (every system would include it, and one change would rebuild everything). Never `#define`, never `static const` in a header.
- **A header includes only what it needs and forward declares the rest.** A type used only through a pointer, a reference, or a declared function's signature gets `class Window;`, and the `.cpp` includes the header. A member held by value, a base class, inline code and templates need the include. Never forward declare `std` or third-party types (undefined behavior; a library may turn a class into an alias). Don't turn a value member into a pointer to save an include; where a header must hide its members, use a private `Impl`.
- **Shaders are embedded, never read at run time.** `dilithium_add_shaders(<target> NAMESPACE <ns> FILES <glsl...>)` compiles each with glslc (`-Werror`, Vulkan 1.3) into `<build>/shaders/<name>_<stage>.hpp`, holding `inline constexpr std::array<uint32_t, N> k<Name><Stage>`; include it as `"shaders/<name>_<stage>.hpp"`. A GLSL error fails the build; a shader edit rebuilds only its header and what includes it. A game calls the same function on its own shaders; an engine shader a game never uses never reaches its binary.
- **Numeric types.** `float` for all math (positions, angles, time, colors, matrices); `double` only where a clock or accumulator needs the range, converted back once. `-Wdouble-promotion` catches a float silently widening (`1.0` for `1.0f`, the `double` overload of `std::sin`). Signed `int` for our own counts and indices (unsigned wraps at zero: `size - 1`, `a - b`); `uint32_t` stays inside the Vulkan wrappers, `size_t` at the standard library's edge (`std::ssize` where a signed count is wanted), and `-Wsign-conversion` makes every crossing explicit. Vectors and matrices: glm with intrinsics on, decided at D2.
- **Linted.** clang-tidy runs on every debug build of our targets, with every finding an error, same as a warning. `.clang-tidy` lists each check that is off with its reason beside it; a finding that is wrong in one place gets `// NOLINT(check-name): reason` on that line, never a wider exemption. The checker reads the debug configuration, so an include used only inside `#ifndef DILITHIUM_DEBUG` goes inside that block.
- Strict warnings as errors, sanitizers, clang-tidy and library hardening apply to our targets only (`dilithium_apply_guard_rails`), never to a game that links the engine.
- `v[i]` is checked: libc++ hardening is on (every check in debug, the cheap ones in release), so no `.at()` for bounds safety. Debug builds also run UBSan's `implicit-conversion`, `float-divide-by-zero`, `nullability` and `local-bounds` groups, and any report aborts.
- `.gitignore` drops any folder named `debug/`, `release/`, `bin/`, `obj/` or `log/`. Never name a source folder that.
- Sources and tests are globbed: adding a file needs no CMake edit, but any `.cpp` under `engine/src/` and any `*Tests.cpp` under `tests/` gets built. Keep experiments elsewhere.

## Build & test

```bash
cmake --preset debug && cmake --build --preset debug && ctest --preset debug
./build/debug/demos/d01_clear_screen/d01_clear_screen                         # the window; quit with Cmd-Q
./build/debug/demos/d02_triangle/d02_triangle                                 # a turning red-green-blue triangle, red up
./build/debug/tests/stress/stress_run --frames 600                            # the engine's proof: exit 0, no validation output
```

- Tools, pinned, installed once per machine (standalone builds from PyPI; Apple's clang stays the compiler):
  ```bash
  uv tool install clang-tidy==22.1.8 && uv tool install clang-format==23.1.1   # both land in ~/.local/bin
  ```
  A debug configure fails with that command if clang-tidy is missing or another version. Upgrading either is a commit of its own: new pin in `cmake/Lint.cmake`, `tools/git-hooks/pre-commit` and here, fix the new findings.
- Architecture diagrams: `tools/diagrams.sh` regenerates `docs/architecture/generated/` from the debug build's compile commands (`brew install clang-uml`; it pulls `llvm@22`, kept out of the `PATH`). Run it after an API change and commit the result; `.clang-uml` says what each diagram includes.
- Pre-commit hook, turned on once per clone with `git config core.hooksPath tools/git-hooks`: the staged C++ files must be clang-formatted (checked on the staged bytes) and pass clang-tidy (staged `.cpp` files; every `.cpp` if a header is staged). It needs `build/debug` configured and fails loudly without it. Never `git commit --no-verify`.
- `debug`: Ninja, `build/debug`, ASan + UBSan (`DILITHIUM_SANITIZE=ON`), `DILITHIUM_DEBUG=1`, libc++ hardening DEBUG, clang-tidy on every file. `release`: `build/release`, no sanitizers, no lint, hardening FAST, `_FORTIFY_SOURCE=3`, stack protector. CMake's own dev and deprecation warnings are errors in both.
- `DILITHIUM_BUILD_DEMOS` / `DILITHIUM_BUILD_TESTS` default on only when this is the top-level project.
- In-source builds are refused.
- Every program understands one engine flag: `--frames N` quits after N loop ticks (exit 1 if validation reported anything). Anything else on the command line is the game's.
- The stress run (`tests/stress/stress_run`) is a program, not a unit test: its scene resizes the window every 20 frames and minimizes it every 97, restoring as soon as the engine reports the background state, while drawing a cycling clear and a turning triangle. `--frames 600` on both presets is the proof after every engine change. The engine knows nothing about it.
- A Catch2 test name must not start with `-`: ctest passes the name on the command line, where Catch2 reads it as a flag.

**Validation:** debug builds run the Khronos validation layer with two extra checks on through `VK_EXT_layer_settings`: synchronization validation (`validate_sync`), which checks that barriers and semaphores really order the GPU's work, and best-practices validation (`validate_best_practices`), which warns about legal but poor use. Both count toward the zero-message rule. A validation error logs `error: validation: …` and aborts at the call. `DILITHIUM_NO_VALIDATION=1` turns it off, loudly. Brew's layer manifest names its library by bare file name, so a program must have `/opt/homebrew/lib` in its search paths (CMake adds it to anything linking the engine); without it the layer fails to load, loudly. `VK_LOADER_DEBUG=layer` shows whether the loader inserted it.

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
