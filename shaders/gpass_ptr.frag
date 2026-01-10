#version 450
#pragma shader_stage(fragment)
#extension GL_EXT_nonuniform_qualifier: enable
#extension GL_EXT_buffer_reference: require
#extension GL_EXT_shader_explicit_arithmetic_types_int64: require
#extension GL_EXT_shader_8bit_storage: require
#extension GL_EXT_shader_explicit_arithmetic_types: require
#extension GL_GOOGLE_include_directive: require
#extension GL_EXT_spec_constant_composites: require

#include "shared_structs.glsl"

layout (set = 2, binding = 0) uniform sampler shardedSampler;
layout (set = 2, binding = 1) uniform texture2D textures[];

layout (location = 0) in vec2 fragTexCoord;
layout (location = 1) in vec3 objectNormal;
layout (location = 2) in vec3 outTangent;
layout (location = 3) in flat uint model_id;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec2 outNormal;
layout (location = 2) out vec4 outMetalRougness;

layout (push_constant) uniform PushConstant {
    ModelInfoReference model_data_pointer;
} push_constants;

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

vec3 DecodeBC5Normal(vec2 rg) {
    vec3 normal;
    normal.xy = rg * 2.0 - 1.0;  // Remap from [0,1] to [-1,1]
    normal.z = sqrt(1.0 - clamp(dot(normal.xy, normal.xy), 0.0, 1.0));
    return normalize(normal);
}

void main() {
    ModelInfo model_info = push_constants.model_data_pointer.model_data[model_id];
    vec4 color = texture(sampler2D(textures[model_info.diffuse_texture_index], shardedSampler), fragTexCoord).rgba;

    if (color.a < 0.95) {
        discard;
    }
    outColor = color;

    vec4 roughness_metal = texture(sampler2D(textures[model_info.metalness_texture_index], shardedSampler), fragTexCoord).rgba;
    outMetalRougness.rg = vec2(roughness_metal.g, roughness_metal.b);


    vec3 normal_texture;
    if (model_info.decompressed_normals) {
        normal_texture = DecodeBC5Normal(texture(sampler2D(textures[model_info.normal_texture_index], shardedSampler), fragTexCoord).rg);
    }
    else {
        normal_texture = texture(sampler2D(textures[model_info.normal_texture_index], shardedSampler), fragTexCoord).rgb;
        normal_texture = (2.0f * normal_texture) - 1.0f;
    }

    const vec3 binormal = cross(objectNormal, outTangent);

    mat4 tagentSpace = mat4(
    vec4(outTangent, 0.0f),
    vec4(binormal, 0.0f),
    vec4(objectNormal, 0.0f),
    vec4(0.0f)
    );

    const vec3 normal = vec3(tagentSpace * vec4(normal_texture, 0.0f));

    outNormal = EncodeNormalOcta(normal);
}
