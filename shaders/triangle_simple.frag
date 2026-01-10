#version 450
#pragma shader_stage(fragment)
#extension GL_EXT_spec_constant_composites: enable


layout (location = 0) in vec3 color;
layout (location = 0) out vec4 outColor;

void main() {
    outColor = vec4(color, 1.0);
}