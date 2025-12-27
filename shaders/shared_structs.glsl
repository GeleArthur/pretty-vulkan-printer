#ifndef SHARED
#define SHARED

#extension GL_EXT_shader_explicit_arithmetic_types_int64: require

//bool VisibleFrustumCone(vec4 sphere)
//{
//    // Cone and sphere are within intersectable range
//    vec3 v0 = sphere.xyz - sceneInfo.camera_frustom.Tip;
//    //    float d0 = dot(v0, sceneInfo.camera_frustom.Direction);
//    //    bool i0 = (d0 <= (sceneInfo.camera_frustom.Height + sphere.w));
//
//    float diff = dot(normalize(v0), normalize(sceneInfo.camera_frustom.Direction));
//    bool inside = diff >= cos(sceneInfo.camera_frustom.Angle);
//
//    // TODO: Find source for this code.
//    //    float cs = cos(sceneInfo.camera_frustom.Angle * 0.5);
//    //    float sn = sin(sceneInfo.camera_frustom.Angle * 0.5);
//    //    float a = dot(v0, sceneInfo.camera_frustom.Direction);
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
    vec3 Position;
    vec2 TexCoord;
    vec3 Normal;
    vec3 Tangent;
};

struct Meshlet {
    uint VertexOffset;
    uint TriangleOffset;
    uint VertexCount;
    uint TriangleCount;
};

struct DrawCommand
{
    uint groupCountX;
    uint groupCountY;
    uint groupCountZ;
    uint meshlet_offset;
    uint meshlet_count;
};

#define AS_GROUP_SIZE 32
struct Payload {
    uint MeshletIndices[AS_GROUP_SIZE];
    bool visable[AS_GROUP_SIZE];
    uint model_matrix_id;
};

struct FrustumCone
{
    vec3 Tip;
    float Height;
    vec3 Direction;
    float Angle;
};

struct ConeBounds {
    vec4 sphereBounds;
    vec4 coneAxis;
};

struct SceneGlobals {
    mat4x4 camera_view;
    mat4x4 camera_projection;
    vec3 position;
    FrustumCone camera_frustom;
};

layout (std430, buffer_reference, buffer_reference_align = 16) buffer VertexReference {
    Vertex vetexData[];
};

layout (std430, buffer_reference, buffer_reference_align = 16) buffer MeshLetReference {
    Meshlet meshLetData[];
};

layout (std430, buffer_reference, buffer_reference_align = 16) buffer TriangleIndicesReference {
    uint8_t triangleIndicesData[];
};

layout (std430, buffer_reference, buffer_reference_align = 16) buffer MeshLetVertexReference {
    uint MeshletVertexData[];
};

layout (std430, buffer_reference, buffer_reference_align = 32) buffer ConeDataReference {
    ConeBounds coneData[];
};

layout (std430, buffer_reference, buffer_reference_align = 64) buffer MatrixReference {
    mat4 model_matrix[];
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