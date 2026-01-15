#extension GL_GOOGLE_cpp_style_line_directive: require

#ifndef SHARED
#define SHARED

//bool VisibleFrustumCone(vec4 sphere)
//{
//    // Cone and sphere are within intersectable range
//    vec3 v0 = sphere.xyz - sceneInfo.camera_frustom.tip;
//    //    float d0 = dot(v0, sceneInfo.camera_frustom.direction);
//    //    bool i0 = (d0 <= (sceneInfo.camera_frustom.height + sphere.w));
//
//    float diff = dot(normalize(v0), normalize(sceneInfo.camera_frustom.direction));
//    bool inside = diff >= cos(sceneInfo.camera_frustom.angle);
//
//    // TODO: Find source for this code.
//    //    float cs = cos(sceneInfo.camera_frustom.angle * 0.5);
//    //    float sn = sin(sceneInfo.camera_frustom.angle * 0.5);
//    //    float a = dot(v0, sceneInfo.camera_frustom.direction);
//    //    float b = a * sn / cs;
//    //    float c = sqrt(dot(v0, v0) - (a * a));
//    //    float d = c - b;
//    //    float e = d * cs;
//    //    bool i1 = (e < sphere.w);
//    return inside;
//
//    //    return i0 && inside; //&& i1;
//}

struct Vertex {
    vec3 position;
    vec2 tex_coord;
    vec3 normal;
    vec3 tangent;
};

struct Meshlet {
    uint vertex_offset;
    uint triangle_offset;
    uint vertex_count;
    uint triangle_count;
};

struct DrawCommand
{
    uint group_count_x;
    uint group_count_y;
    uint group_count_z;
    uint meshlet_offset;
    uint meshlet_count;
};

#define AS_GROUP_SIZE 32
struct Payload {
    uint meshlet_indices[AS_GROUP_SIZE];
    uint model_index;
    mat4 full_matrix;
};

struct FrustumCone
{
    vec3 tip;
    float height;
    vec3 direction;
    float angle;
};

struct RadarCull
{
    vec3 camera_x;
    float far_plane;
    vec3 camera_y;
    float near_plane;
    vec3 camera_z;
    float tang;
    float ratio;
    float sphere_factor_y;
    float sphere_factor_x;
};

struct ConeBounds {
    vec4 sphere_bounds;
    vec4 cone_axis;
};

struct SceneGlobals {
    mat4x4 camera_view;
    mat4x4 camera_projection;
    vec3 position;
    FrustumCone camera_frustom;
    mat4x4 camera_projection_view;
    RadarCull rader_cull;
    int cull_mode;
};

struct ModelInfo {
    mat4 model;
    mat4 model_view_projection;
    uint diffuse_texture_index;
    uint normal_texture_index;
    uint metalness_texture_index;
    bool decompressed_normals;
};

layout (std430, buffer_reference, buffer_reference_align = 8) buffer VertexReference {
    Vertex vertex_data[];
};

layout (std430, buffer_reference, buffer_reference_align = 8) buffer MeshLetReference {
    Meshlet meshlet_data[];
};

layout (std430, buffer_reference, buffer_reference_align = 8) buffer TriangleIndicesReference {
    uint8_t triangle_indices_data[];
};

layout (std430, buffer_reference, buffer_reference_align = 8) buffer MeshLetVertexReference {
    uint meshlet_vertex_data[];
};

layout (std430, buffer_reference, buffer_reference_align = 8) buffer ConeDataReference {
    ConeBounds cone_data[];
};

layout (std430, buffer_reference, buffer_reference_align = 8) buffer ModelInfoReference {
    ModelInfo model_data[];
};

layout (std430, buffer_reference, buffer_reference_align = 8) buffer MatrixFullReference {
    mat4 model_data[];
};

struct MeshletsBuffers
{
    VertexReference vertex_data;
    MeshLetReference meshlet_data;
    MeshLetVertexReference meshlet_vertices_data;
    TriangleIndicesReference meshlet_triangle_data;
    ConeDataReference meshlet_sphere_bounds_data;
};

uint hash(uint a)
{
    a = (a + 0x7ed55d16) + (a << 12);
    a = (a ^ 0xc761c23c) ^ (a >> 19);
    a = (a + 0x165667b1) + (a << 5);
    a = (a + 0xd3a2646c) ^ (a << 9);
    a = (a + 0xfd7046c5) + (a << 3);
    a = (a ^ 0xb55a4f09) ^ (a >> 16);
    return a;
}


#endif