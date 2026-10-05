#pragma once

#include <dilithium/scenes/Scene.hpp>

namespace dilithium {
class Renderer;
struct SceneServices;
} // namespace dilithium

namespace demo {

/// D2: one triangle, a corner each of red, green and blue, turning once every ten seconds. Red starts at the top:
/// if it is at the bottom, +Y is pointing the wrong way. Turning proves frames advance and that the vertex buffer
/// is rewritten every frame without the GPU reading a half-written one.
class TriangleScene final : public dilithium::Scene {
public:
    explicit TriangleScene(dilithium::SceneServices& services);

    void update(float dt) override;
    void draw() override;

private:
    dilithium::Renderer& renderer;
    float angle = 0.0f;
};

} // namespace demo
