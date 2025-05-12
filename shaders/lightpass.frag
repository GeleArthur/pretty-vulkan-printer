#version 450

layout (binding = 0) uniform sampler2D albedoImage;
layout (binding = 1) uniform sampler2D normalImage;

//layout(std140, set = 0, binding = 2) uniform ScreenUBO{
//    vec2 screensize;
//} uScreen;

layout (location = 0) out vec4 outColor;

void main() {
    outColor = texture(albedoImage, gl_FragCoord.xy / vec2(800, 600)) * texture(normalImage, gl_FragCoord.xy / vec2(800, 600));
}