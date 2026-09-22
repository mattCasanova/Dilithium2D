# Dilithium2D

A 2D game engine in C++20 and raw Vulkan 1.3, built for fun and to learn Vulkan. It follows the design of [LiquidMetal2D](https://github.com/mattCasanova/LiquidMetal2D), a Swift/Metal engine: a renderer that could do 3D, used for 2D games, grown one demo at a time.

Status: just started. The first demo, a window that clears to a color, is under way.

## Build

You need CMake 3.25+, Ninja, a C++20 compiler, and the Vulkan loader, headers and validation layers. On macOS, with Homebrew:

```bash
brew install cmake ninja vulkan-headers vulkan-loader molten-vk vulkan-validationlayers vulkan-tools shaderc sdl3
```

Then:

```bash
cmake --preset debug           # Debug, with AddressSanitizer + UndefinedBehaviorSanitizer
cmake --build --preset debug
ctest --preset debug
```

The `release` preset works the same way, without sanitizers. The other dependencies (VMA, glm, nlohmann/json, Catch2, and SDL3 when it is not installed) download during the first configure.

Render long and prosper.
