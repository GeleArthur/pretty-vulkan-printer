#ifndef WORLD_BINDS
#define WORLD_BINDS

#include "shared_structs.glsl"


layout (set = 0, binding = 0) uniform SceneGlobals {
    mat4x4 camera_view;
    mat4x4 camera_projection;
    vec3 position;
    FrustumCone camera_frustom;
} sceneInfo;

bool VisibleFrustumCone(vec4 sphere)
{
    // Cone and sphere are within intersectable range
    vec3 v0 = sphere.xyz - sceneInfo.camera_frustom.Tip;
    float d0 = dot(v0, sceneInfo.camera_frustom.Direction);
    bool i0 = (d0 <= (sceneInfo.camera_frustom.Height + sphere.w));

    //    float diff = dot(normalize(v0), normalize(sceneInfo.camera_frustom.Direction));
    //    bool inside = diff >= cos(sceneInfo.camera_frustom.Angle);

    // TODO: Find source for this code.
    float cs = cos(sceneInfo.camera_frustom.Angle);
    float sn = sin(sceneInfo.camera_frustom.Angle);
    float a = dot(v0, sceneInfo.camera_frustom.Direction);
    float b = a * sn / cs;
    float c = sqrt(dot(v0, v0) - (a * a));
    float d = c - b;
    float e = d * cs;
    bool i1 = (e < sphere.w);

    return i0 && i1;
}
#endif