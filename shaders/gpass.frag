#version 450

layout (location = 0) in vec2 fragTexCoord;
layout (location = 1) in vec3 objectNormal;


//layout (binding = 1) uniform sampler2D texSampler;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outNormal;


// Helper: wrap across octahedron edges
vec2 OctWrap(vec2 v) {
    // (1 - abs(yx)) * sign‐vector
    return (1.0 - abs(v.yx)) * vec2(v.x >= 0.0 ?  1.0 : -1.0, v.y >= 0.0 ?  1.0 : -1.0);
}

// Encode a unit normal into 2D octahedral coords
vec2 EncodeNormalOcta(vec3 n) {
    // 1) Project onto octahedron
    n /= (abs(n.x) + abs(n.y) + abs(n.z));
    // 2) If the original Z was negative, wrap
    if (n.z < 0.0) {
        n.xy = OctWrap(n.xy);
    }
    // 3) Remap from [-1,1] to [0,1]
    return n.xy * 0.5 + 0.5;
}

void main() {
    outColor = vec4(fragTexCoord.x, fragTexCoord.y, 0.0, 1.0);

    vec2 normalEncoded = EncodeNormalOcta(objectNormal);
    outNormal = vec4(normalEncoded.x, normalEncoded.y, 0.0, 0.0);
}