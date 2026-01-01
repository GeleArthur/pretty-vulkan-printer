#version 450
#pragma shader_stage(fragment)
#extension GL_EXT_nonuniform_qualifier: enable

#include "shared_structs.glsl"

layout (set = 2, binding = 0) uniform sampler shardedSampler;
layout (set = 2, binding = 1) uniform texture2D textures[];

layout (location = 0) in vec2 fragTexCoord;
layout (location = 1) in flat uint model_id;

layout (push_constant) uniform PushConstant {
    ModelInfoReference model_data_pointer;
} push_constants;

void main() {
    uint diffuse_index = push_constants.model_data_pointer.model_data[model_id].diffuse_texture_index;

    vec4 color = texture(sampler2D(textures[diffuse_index], shardedSampler), fragTexCoord).rgba;

    if (color.a < 0.95) {
        discard;
    }
}
