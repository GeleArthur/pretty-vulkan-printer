#ifndef WORLD_BINDS
#define WORLD_BINDS

#include "shared_structs.glsl"


layout (set = 0, binding = 0) uniform SceneGlobalsIn {
    SceneGlobals sceneInfo;
};



bool VisibleFrustumCone(vec4 sphere)
{
    // Cone and sphere are within intersectable range
    vec3 v0 = sphere.xyz - sceneInfo.camera_frustom.tip;
    float d0 = dot(v0, sceneInfo.camera_frustom.direction);
    bool i0 = (d0 <= (sceneInfo.camera_frustom.height + sphere.w));

    //    float diff = dot(normalize(v0), normalize(sceneInfo.camera_frustom.direction));
    //    bool inside = diff >= cos(sceneInfo.camera_frustom.angle);

    // TODO: Find source for this code.
    float cs = cos(sceneInfo.camera_frustom.angle);
    float sn = sin(sceneInfo.camera_frustom.angle);
    float a = dot(v0, sceneInfo.camera_frustom.direction);
    float b = a * sn / cs;
    float c = sqrt(dot(v0, v0) - (a * a));
    float d = c - b;
    float e = d * cs;
    bool i1 = (e < sphere.w);

    return i0 && i1;
}

bool BackfaceCulling(ConeBounds cone) {
    return dot(normalize(cone.sphere_bounds.xyz - sceneInfo.camera_frustom.tip), cone.cone_axis.xyz) >= cone.cone_axis.w;
}


bool IsVisible(ConeBounds cone) {
    if (!VisibleFrustumCone(cone.sphere_bounds)) {
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

    cone.sphere_bounds = vec4(
    vec3(matrix * vec4(cone.sphere_bounds.xyz, 1.0f)),
    cone.sphere_bounds.w * maxScale
    );

    cone.cone_axis.xyz = mat3(matrix) * cone.cone_axis.xyz;

    return cone;
}

#endif