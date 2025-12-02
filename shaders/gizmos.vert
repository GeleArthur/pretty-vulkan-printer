#version 450
#pragma shader_stage(vertex)

layout (set = 0, binding = 0) uniform SceneGlobals {
    mat4x4 camera_view;
    mat4x4 camera_projection;
} sceneInfo;

layout (push_constant) uniform PushConstant {
    mat4 model;
    uint diffuse_texture_index;
    uint normal_texture_index;
    uint metalness_texture_index;
} pc;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec4 inColor;

layout (location = 0) out vec4 color;

void main()
{
    gl_Position = sceneInfo.camera_projection * sceneInfo.camera_view * pc.model * vec4(inPosition, 1.0);
    color = inColor;
}
