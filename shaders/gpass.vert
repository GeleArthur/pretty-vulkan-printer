#version 450

layout (set = 0, binding = 0) uniform SceneGlobals {
    mat4x4 camera_view_projection;
    vec3   lights[1];
} sceneInfo;


layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inTexCoord;
layout (location = 2) in vec3 inNormal;

layout (location = 0) out vec2 fragTexCoord;
layout (location = 1) out vec3 normalCoord;

void main() {
    gl_Position = sceneInfo.camera_view_projection * vec4(inPosition, 1.0);
    fragTexCoord = inTexCoord;
    normalCoord = vec3(sceneInfo.camera_view_projection * vec4(inNormal, 0.0));
}
