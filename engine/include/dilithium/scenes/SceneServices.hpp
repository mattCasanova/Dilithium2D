#pragma once

namespace dilithium {

class Renderer;
class SceneManager;

/// What a scene's constructor receives: the engine's services, by reference, for the scene's whole life.
/// LiquidMetal2D's `SceneServices`. Input (D9) and audio (D10) join it later. A scene that needs something only a
/// particular `Renderer` implementation has downcasts, as LiquidMetal2D's scenes do with `services as? GameServices`.
struct SceneServices {
    Renderer& renderer;
    SceneManager& scenes;
};

} // namespace dilithium
