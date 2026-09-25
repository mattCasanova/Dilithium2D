#pragma once

#include <dilithium/Color.hpp>
#include <dilithium/NonCopyable.hpp>

#include <memory>
#include <string>

namespace dilithium {

/// The window a program starts with.
struct AppConfig {
    std::string title = "Dilithium2D";
    int width = 1280; ///< in points, not pixels: on a high-density display the window has more pixels than this
    int height = 720;
};

/// What a game or demo implements. The engine calls it; it never sees SDL or Vulkan.
class App : NonCopyable {
public:
    virtual ~App() = default;

    virtual AppConfig config() const { return {}; }
    virtual void onStart() {}
    virtual void onUpdate(float dt) = 0;  ///< `dt` in seconds, clamped to 1/15 like LiquidMetal2D
    virtual Color clearColor() const = 0; ///< D1 only; real drawing replaces it in D2
    virtual void onShutdown() {}
};

/// Defined by the game, once per executable. The engine's entry point (`dilithium::main`) calls it at start-up.
std::unique_ptr<App> createApp(int argc, char** argv);

} // namespace dilithium
