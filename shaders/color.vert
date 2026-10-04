#version 450

// D2: a position and a color per vertex, straight through. Positions are already in clip space; D5's camera adds
// the projection. The viewport has a negative height (see RenderCore), so +Y is up here, as in LiquidMetal2D.

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec4 fragColor;

void main() {
    gl_Position = vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
}
