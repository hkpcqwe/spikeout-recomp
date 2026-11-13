// A translucent overlay (touch controls) drawn over the swapchain: sample
// keeps alpha so the presenter's blend state can composite it.
#version 450

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 colour;

layout(set = 2, binding = 0) uniform sampler2D frame;

void main() {
    colour = texture(frame, uv);
}
