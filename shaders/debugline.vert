#version 450
#pragma shader_stage(vertex)

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec4 inColor;

layout (location = 0) out vec4 outColor;

layout (set = 0, binding = 0) uniform SceneGlobals {
    mat4x4 camera_view;
    mat4x4 camera_projection;
    vec3 position;
} sceneInfo;

void main()
{
    gl_Position = sceneInfo.camera_projection * sceneInfo.camera_view * vec4(inPosition, 1.0);
    outColor = inColor;
}