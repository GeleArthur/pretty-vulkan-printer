#version 450

layout (location = 0) in vec2 fragTexCoord;
layout (location = 1) in vec3 objectNormal;


//layout (binding = 1) uniform sampler2D texSampler;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outNormal;

void main() {
    outColor = vec4(fragTexCoord.x, fragTexCoord.y, 0.0, 1.0);
    outNormal = vec4(objectNormal.x, objectNormal.y, 1.0, 1.0);
}