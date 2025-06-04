#version 450
#pragma shader_stage(fragment)
#extension GL_EXT_nonuniform_qualifier: enable

layout (set = 1, binding = 0) uniform sampler shardedSampler;
layout (set = 1, binding = 1) uniform texture2D textures[];

layout (location = 0) in vec2 fragTexCoord;
layout (location = 1) in vec3 objectNormal;
layout (location = 2) in vec3 outTangent;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outNormalAndMetalRougness;

layout (push_constant) uniform PushConstant {
    mat4 model;
    uint diffuse_texture_index;
    uint normal_texture_index;
    uint metalness_texture_index;
} pc;

// Helper: wrap across octahedron edges
vec2 OctWrap(vec2 v) {
    // (1 - abs(yx)) * sign‐vector
    return (1.0 - abs(v.yx)) * vec2(v.x >= 0.0 ? 1.0 : -1.0, v.y >= 0.0 ? 1.0 : -1.0);
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

    if (color.a < 0.95) {
        discard;
    }

    vec4 roughness_metal = texture(sampler2D(textures[pc.metalness_texture_index], shardedSampler), fragTexCoord).rgba;
    vec3 normal_texture = texture(sampler2D(textures[pc.normal_texture_index], shardedSampler), fragTexCoord).rgb;

    normal_texture = (2.0f * normal_texture) - 1.0f;

    outColor = color;

    const vec3 binormal = cross(objectNormal, outTangent);

    mat4 tagentSpace = mat4(
    vec4(outTangent, 0.0f),
    vec4(binormal, 0.0f),
    vec4(objectNormal, 0.0f),
    vec4(0.0f)
    );

    const vec3 normal = vec3(tagentSpace * vec4(normal_texture, 0.0f));

    vec2 normalEncoded = EncodeNormalOcta(normal);
    outNormalAndMetalRougness = vec4(normalEncoded.x, normalEncoded.y, roughness_metal.g, roughness_metal.b);
}
