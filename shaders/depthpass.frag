#version 450
#pragma shader_stage(fragment)
#extension GL_EXT_nonuniform_qualifier: enable

layout (set = 1, binding = 0) uniform sampler shardedSampler;
layout (set = 1, binding = 1) uniform texture2D textures[];

layout (location = 0) in vec2 fragTexCoord;

layout (push_constant) uniform PushConstant {
    mat4 model;
    uint diffuse_texture_index;
    uint normal_texture_index;
    uint metalness_texture_index;
} pc;

void main() {
    vec4 color = texture(sampler2D(textures[pc.diffuse_texture_index], shardedSampler), fragTexCoord).rgba;

    if (color.a < 0.95) {
        discard;
    }
}
