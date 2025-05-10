#version 450

layout (binding = 0) uniform sampler2D albedoImage;
layout (binding = 1) uniform sampler2D normalImage;

layout (location = 0) out vec4 outColor;

void main() {
    outColor = vec4(gl_FragCoord.x, gl_FragCoord.y, 1.0, 1.0);
}