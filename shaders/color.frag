#version 450

// The interpolated vertex color, unchanged, into the swapchain's B8G8R8A8_UNORM image: the same values look the
// same as in LiquidMetal2D.

layout(location = 0) in vec4 fragColor;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = fragColor;
}
