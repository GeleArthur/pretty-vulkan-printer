#extension GL_GOOGLE_include_directive: require
#extension GL_EXT_spec_constant_composites: require

#ifndef WORLD_BINDS
#define WORLD_BINDS

#include "shared_structs.glsl"


layout (set = 0, binding = 0) uniform SceneGlobalsIn {
    SceneGlobals sceneInfo;
};


bool VisibleFrustumCone(vec4 sphere) {
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

bool RaderCulling(ConeBounds cone) {
    bool result = true;
    vec3 ray = cone.sphere_bounds.xyz - sceneInfo.position;
    float z_projections = dot(ray, sceneInfo.rader_cull.camera_z);

    if (z_projections > sceneInfo.rader_cull.far_plane + cone.sphere_bounds.w || z_projections < sceneInfo.rader_cull.near_plane - cone.sphere_bounds.w) {
        return false;
    }
    //    if (z_projections > sceneInfo.rader_cull.far_plane - cone.sphere_bounds.w || z_projections < sceneInfo.rader_cull.near_plane + cone.sphere_bounds.w) {
    //        result = true;
    //    }

    float y_projections = dot(ray, sceneInfo.rader_cull.camera_y);
    float sphere_size_y = sceneInfo.rader_cull.sphere_factor_y * cone.sphere_bounds.w;
    float y_height = z_projections * sceneInfo.rader_cull.tang;
    if (y_projections > y_height + sphere_size_y || y_projections < -y_height - sphere_size_y) {
        return false;
    }
    //    if (y_projections > x_height - sphere_size_y || y_projections < -x_height + sphere_size_y) {
    //        result = true;
    //    }


    float x_projection = dot(ray, sceneInfo.rader_cull.camera_x);
    float x_height = y_height * sceneInfo.rader_cull.ratio;
    float sphere_size_x = sceneInfo.rader_cull.sphere_factor_x * cone.sphere_bounds.w;
    if (x_projection > x_height + sphere_size_x || x_projection < -x_height - sphere_size_x) {
        return false;
    }

    //    if (x_projection > y_height - sphere_size_x || x_projection < -y_height + sphere_size_x) {
    //        result = true;
    //    }

    return result;
}

bool IsVisible(ConeBounds cone) {
    if (sceneInfo.cull_mode == 0) return true;
    if (sceneInfo.cull_mode >= 1) {
        if (BackfaceCulling(cone)) {
            return false;
        }
    }
    if (sceneInfo.cull_mode == 2) {
        if (!RaderCulling(cone)) {
            return false;
        }
    }
    else if (sceneInfo.cull_mode == 3) {
        if (!VisibleFrustumCone(cone.sphere_bounds)) {
            return false;
        }
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