#ifndef WORLD_BINDS
#define WORLD_BINDS

#include "shared_structs.glsl"


layout (set = 0, binding = 0) uniform SceneGlobalsIn {
    SceneGlobals sceneInfo;
};



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

bool BackfaceCulling(ConeBounds cone) {
    return dot(normalize(cone.sphereBounds.xyz - sceneInfo.camera_frustom.Tip), cone.coneAxis.xyz) >= cone.coneAxis.w;
}


bool IsVisible(ConeBounds cone) {
    if (!VisibleFrustumCone(cone.sphereBounds)) {
        return false;

    }
    if (BackfaceCulling(cone)) {
        return false;
    }
    return true;
}

ConeBounds TransformCone(ConeBounds cone, mat4 matrix) {
    vec3 scale = vec3(
    length(matrix[0].xyz),
    length(matrix[1].xyz),
    length(matrix[2].xyz)
    );
    float maxScale = max(max(scale.x, scale.y), scale.z);

    cone.sphereBounds = vec4(
    vec3(matrix * vec4(cone.sphereBounds.xyz, 1.0f)),
    cone.sphereBounds.w * maxScale
    );

    cone.coneAxis.xyz = mat3(matrix) * cone.coneAxis.xyz;

    return cone;
}

#endif