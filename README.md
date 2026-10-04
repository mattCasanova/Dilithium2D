# Dilithium2D

A 2D game engine in C++20 and raw Vulkan 1.3, built for fun and to learn Vulkan. It follows the design of [LiquidMetal2D](https://github.com/mattCasanova/LiquidMetal2D), a Swift/Metal engine: a renderer that could do 3D, used for 2D games, grown one demo at a time.

Status: the first demo works: a window that clears to a color, with Vulkan validation (synchronization checks included) reporting nothing through resizes and minimizes.

## Build

You need CMake 3.25+, Ninja, a C++20 compiler, and the Vulkan loader, headers and validation layers. On macOS, with Homebrew:

```bash
brew install cmake ninja vulkan-headers vulkan-loader molten-vk vulkan-validationlayers vulkan-tools shaderc sdl3
```

Debug builds lint with clang-tidy and the pre-commit hook checks formatting and lint, both at pinned versions:

```bash
uv tool install clang-tidy==22.1.8 && uv tool install clang-format==23.1.1   # once per machine
git config core.hooksPath tools/git-hooks                                     # once per clone
```

Then:

```bash
cmake --preset debug           # Debug, with AddressSanitizer + UndefinedBehaviorSanitizer, clang-tidy on every file
cmake --build --preset debug
ctest --preset debug
```

The `release` preset works the same way, without sanitizers. The other dependencies (VMA, glm, nlohmann/json, Catch2, and SDL3 when it is not installed) download during the first configure.

Then run the first demo, a window that clears to a slowly cycling color:

```bash
./build/debug/demos/d01_clear_screen/d01_clear_screen                         # quit with Cmd-Q
./build/debug/demos/d01_clear_screen/d01_clear_screen --frames 600 --torture  # stress run: resizes, minimizes, quits
```

## Writing a game

A game is scenes plus one function. A scene subclasses `dilithium::Scene`: its constructor is the setup, its destructor the shutdown, and it gets `update(dt)` and `draw()` once per frame. The function is `dilithium::createEngine`, which builds the window and the renderer, hands both to the engine, and registers the scenes:

```cpp
#include <dilithium/engine/Engine.hpp>
#include <dilithium/gfx/renderers/DefaultRenderer.hpp>
#include <dilithium/platform/Window.hpp>
#include <dilithium/scenes/Scene.hpp>
#include <dilithium/scenes/SceneManager.hpp>
#include <dilithium/scenes/SceneServices.hpp>

enum class SceneId { Menu };

class MenuScene final : public dilithium::Scene {
public:
    explicit MenuScene(dilithium::SceneServices& services) : m_renderer(services.renderer) {}
    void update(float dt) override {}
    void draw() override { m_renderer.setClearColor({.r = 0.1f, .g = 0.4f, .b = 0.8f}); }
private:
    dilithium::Renderer& m_renderer;
};

std::unique_ptr<dilithium::Engine> dilithium::createEngine(const CommandLine& commandLine) {
    auto window = std::make_unique<Window>(WindowConfig{.title = "My Game"});
    auto renderer = std::make_unique<DefaultRenderer>(*window);
    auto engine = std::make_unique<Engine>(std::move(window), std::move(renderer), commandLine);
    engine->scenes().add<MenuScene>(SceneId::Menu);
    engine->scenes().start(SceneId::Menu);
    return engine;
}
```

Scenes change screens through `services.scenes`: `set` replaces the current scene, `push` covers it (a pause menu), `pop` returns. Each happens at the start of the next frame. In CMake, pull the engine in with `FetchContent` and link `dilithium::main`; it brings SDL's entry point and the engine with it, and your code never sees an SDL or Vulkan header. `demos/d01_clear_screen/main.cpp` is a complete example.

Render long and prosper.
