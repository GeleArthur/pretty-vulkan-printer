#version 450
#pragma shader_stage(fragment)
#extension GL_EXT_nonuniform_qualifier : enable

layout(set = 1, binding = 0) uniform sampler shardedSampler;
layout(set = 1, binding = 1) uniform texture2D textures[];

layout (location = 0) in vec2 fragTexCoord;
layout (location = 1) in vec3 objectNormal;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outNormalAndMetalRougness;

layout(push_constant) uniform PushConstant {
    mat4 model;
    uint diffuse_texture_index;
    uint normal_texture_index;
    uint metalness_texture_index;
} pc;

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

    vec4 color = texture(sampler2D(textures[pc.diffuse_texture_index], shardedSampler), fragTexCoord).rgba;
    vec4 roughness_metal = texture(sampler2D(textures[pc.metalness_texture_index], shardedSampler), fragTexCoord).rgba;

    outColor = color;

    vec2 normalEncoded = EncodeNormalOcta(objectNormal);
    outNormalAndMetalRougness = vec4(normalEncoded.x, normalEncoded.y, roughness_metal.g, roughness_metal.b);
}
