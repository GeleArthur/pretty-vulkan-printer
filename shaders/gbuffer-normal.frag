#version 450

layout (location = 0) in vec3 objectNormal;
layout (location = 0) out vec4 outColor;

void main() {
    outColor = vec4(objectNormal.x, objectNormal.y, 1.0, 1.0);
}