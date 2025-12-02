#include "ModelData.h"

#include <fstream>
#include <meshoptimizer.h>
#include <numeric>
#include <set>
#include <stb_image.h>
#include <GraphicsPipeline/Vertex.h>
#include <Image/Image.h>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/fwd.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <spdlog/spdlog.h>

namespace
{
    namespace fs = std::filesystem;
    glm::mat4 convert_matrix(const aiMatrix4x4& m)
    {
        glm::mat4 out;
        out[0] = glm::vec4(m.a1, m.b1, m.c1, m.d1);
        out[1] = glm::vec4(m.a2, m.b2, m.c2, m.d2);
        out[2] = glm::vec4(m.a3, m.b3, m.c3, m.d3);
        out[3] = glm::vec4(m.a4, m.b4, m.c4, m.d4);
        return out;
    }

    // TODO: cache on disk
    void generate_meshlet(pvp::ModelData& model_out, const aiMesh* mesh_in)
    {
        constexpr size_t max_vertices = 64;
        constexpr size_t max_triangles = 124;
        constexpr float  cone_weight = 0.0f;

        std::vector<uint8_t> meshlet_triangles_u8;

        size_t max_mesh_lets = meshopt_buildMeshletsBound(mesh_in->mNumFaces * 3, max_vertices, max_triangles);
        model_out.meshlets.resize(max_mesh_lets);
        model_out.meshlet_vertices.resize(max_mesh_lets * max_vertices);
        meshlet_triangles_u8.resize(max_mesh_lets * max_triangles * 3);

        std::vector<float> verticies;
        verticies.reserve(mesh_in->mNumVertices * 3);
        for (size_t i = 0; i < mesh_in->mNumVertices; ++i)
        {
            verticies.push_back(mesh_in->mVertices[i].x);
            verticies.push_back(mesh_in->mVertices[i].y);
            verticies.push_back(mesh_in->mVertices[i].z);
        }

        size_t const meshlet_count = meshopt_buildMeshlets(
            model_out.meshlets.data(),
            model_out.meshlet_vertices.data(),
            meshlet_triangles_u8.data(),
            model_out.indices.data(),
            model_out.indices.size(),
            verticies.data(),
            verticies.size(),
            sizeof(float) * 3,
            max_vertices,
            max_triangles,
            cone_weight);

        auto& last = model_out.meshlets[meshlet_count - 1];
        model_out.meshlet_vertices.resize(last.vertex_offset + last.vertex_count);
        meshlet_triangles_u8.resize(last.triangle_offset + ((last.triangle_count * 3 + 3) & ~3)); // TODO: Understand this? Align 4 bytes
        model_out.meshlets.resize(meshlet_count);

        for (auto& meshlet : model_out.meshlets)
        {
            // Save triangle offset for current meshlet
            uint32_t triangle_offset = static_cast<uint32_t>(model_out.meshlet_triangles.size());

            // Repack to uint32_t
            for (uint32_t i = 0; i < meshlet.triangle_count; ++i)
            {
                uint32_t i0 = 3 * i + 0 + meshlet.triangle_offset;
                uint32_t i1 = 3 * i + 1 + meshlet.triangle_offset;
                uint32_t i2 = 3 * i + 2 + meshlet.triangle_offset;

                uint8_t  vertex_id0 = meshlet_triangles_u8[i0];
                uint8_t  vertex_id1 = meshlet_triangles_u8[i1];
                uint8_t  vertex_id2 = meshlet_triangles_u8[i2];
                uint32_t packed = ((static_cast<uint32_t>(vertex_id0) & 0xFF) << 0) |
                    ((static_cast<uint32_t>(vertex_id1) & 0xFF) << 8) |
                    ((static_cast<uint32_t>(vertex_id2) & 0xFF) << 16);
                model_out.meshlet_triangles.push_back(packed);
            }

            // Update triangle offset for current meshlet
            meshlet.triangle_offset = triangle_offset;
        }

        model_out.meshlet_sphere_bounds.reserve(model_out.meshlets.size());
        for (const auto& meshlet : model_out.meshlets)
        {
            meshopt_Bounds bounds = meshopt_computeMeshletBounds(
                &model_out.meshlet_vertices[meshlet.vertex_offset],
                &meshlet_triangles_u8[meshlet.triangle_offset],
                meshlet.triangle_count,
                verticies.data(),
                verticies.size(),
                sizeof(float) * 3);
            model_out.meshlet_sphere_bounds.push_back(glm::vec4(bounds.center[0], bounds.center[1], bounds.center[2], bounds.radius));
        }
    }

    std::string cached_string(const std::filesystem::path& path)
    {
        return std::format("{}, {:%Y%m%d%H%M}, {}", path.filename().string(), std::filesystem::last_write_time(path), std::filesystem::file_size(path));
    }

    bool has_cache(const std::filesystem::path& path)
    {
        if (!fs::is_directory("cache"))
        {
            fs::create_directory("cache");
        }

        std::string check_name = cached_string(path);

        return fs::exists(fs::path("cache") / check_name);
    }

    // helper functions created with Gemini
    // WRITE
    template<typename T>
    void write_pod(std::ofstream& out, const T& val)
    {
        out.write(reinterpret_cast<const char*>(&val), sizeof(T));
    }

    template<typename T>
    void write_vector(std::ofstream& out, const std::vector<T>& vec)
    {
        uint32_t size = static_cast<uint32_t>(vec.size());
        out.write(reinterpret_cast<const char*>(&size), sizeof(size));
        if (size > 0)
        {
            out.write(reinterpret_cast<const char*>(vec.data()), size * sizeof(T));
        }
    }

    void write_string(std::ofstream& out, const std::string& str)
    {
        uint32_t size = static_cast<uint32_t>(str.size());
        out.write(reinterpret_cast<const char*>(&size), sizeof(size));
        if (size > 0)
        {
            out.write(str.data(), size);
        }
    }

    // READ
    template<typename T>
    void read_pod(std::ifstream& in, T& val)
    {
        in.read(reinterpret_cast<char*>(&val), sizeof(T));
    }

    template<typename T>
    void read_vector(std::ifstream& in, std::vector<T>& vec)
    {
        uint32_t size;
        in.read(reinterpret_cast<char*>(&size), sizeof(size));
        vec.resize(size);
        if (size > 0)
        {
            in.read(reinterpret_cast<char*>(vec.data()), size * sizeof(T));
        }
    }

    void read_string(std::ifstream& in, std::string& str)
    {
        uint32_t size;
        in.read(reinterpret_cast<char*>(&size), sizeof(size));
        str.resize(size);
        if (size > 0)
        {
            in.read(&str[0], size);
        }
    }

    void save_cache(const std::filesystem::path& path, const pvp::LoadedScene& scene)
    {
        std::ofstream out_stream(fs::path("cache") / cached_string(path), std::ios::binary);

        write_pod(out_stream, static_cast<uint32_t>(scene.models.size()));
        for (const pvp::ModelData& model : scene.models)
        {
            write_vector(out_stream, model.vertices);
            write_vector(out_stream, model.indices);
            write_pod(out_stream, model.transform);
            write_string(out_stream, model.diffuse_path);
            write_string(out_stream, model.metallic_path);
            write_string(out_stream, model.normal_path);

            write_vector(out_stream, model.meshlets);
            write_vector(out_stream, model.meshlet_vertices);
            write_vector(out_stream, model.meshlet_triangles);
            write_vector(out_stream, model.meshlet_sphere_bounds);
        }

        auto save_texture = [&](const pvp::TextureData& texture) {
            write_string(out_stream, texture.name);
            write_pod(out_stream, texture.width);
            write_pod(out_stream, texture.height);
            write_pod(out_stream, texture.channels);
            write_pod(out_stream, texture.type);

            uint64_t data_size = texture.width * texture.height * texture.channels;
            out_stream.write(reinterpret_cast<const char*>(texture.pixels), data_size);
        };

        write_pod(out_stream, static_cast<uint32_t>(scene.textures.size()));
        for (const pvp::TextureData& texture : scene.textures)
        {
            save_texture(texture);
        }

        save_texture(scene.cube_map);
    }

    void load_cache(const std::filesystem::path& path, pvp::LoadedScene& scene)
    {
        std::ifstream in_stream(fs::path("cache") / cached_string(path), std::ios::binary);

        uint32_t model_size;
        read_pod(in_stream, model_size);
        scene.models.resize(model_size);

        for (pvp::ModelData& model : scene.models)
        {
            read_vector(in_stream, model.vertices);
            read_vector(in_stream, model.indices);
            read_pod(in_stream, model.transform);
            read_string(in_stream, model.diffuse_path);
            read_string(in_stream, model.metallic_path);
            read_string(in_stream, model.normal_path);

            read_vector(in_stream, model.meshlets);
            read_vector(in_stream, model.meshlet_vertices);
            read_vector(in_stream, model.meshlet_triangles);
            read_vector(in_stream, model.meshlet_sphere_bounds);
        }

        auto load_texture = [&](pvp::TextureData& texture) {
            read_string(in_stream, texture.name);
            read_pod(in_stream, texture.width);
            read_pod(in_stream, texture.height);
            read_pod(in_stream, texture.channels);
            read_pod(in_stream, texture.type);

            const uint64_t data_size = texture.width * texture.height * texture.channels;
            texture.pixels = static_cast<stbi_uc*>(std::malloc(data_size));
            in_stream.read(reinterpret_cast<char*>(texture.pixels), data_size);
        };

        uint32_t texture_count;
        read_pod(in_stream, texture_count);
        scene.textures.resize(texture_count);
        for (pvp::TextureData& texture : scene.textures)
        {
            load_texture(texture);
        }

        load_texture(scene.cube_map);
    }

    pvp::LoadedScene load_scene_from_disk(const std::filesystem::path& path)
    {
        pvp::LoadedScene out_scene{};
        const aiScene*   scene = aiImportFile(path.generic_string().c_str(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_CalcTangentSpace);

        if (scene == nullptr)
        {
            spdlog::error("Failed to load model: {}", aiGetErrorString());
            return out_scene;
        }

        std::unordered_map<std::string, aiTextureType> all_textures;

        std::function<void(const aiNode*, const aiMatrix4x4& parent_transform)> process_node;
        process_node = [&](const aiNode* node, const aiMatrix4x4& parent_transform) {
            const aiMatrix4x4 world_matrix = parent_transform * node->mTransformation;

            for (unsigned int i = 0; i < node->mNumMeshes; ++i)
            {
                const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

                pvp::ModelData model;
                model.vertices.reserve(mesh->mNumVertices);
                model.indices.reserve(mesh->mNumFaces * 3u);
                model.transform = convert_matrix(world_matrix);

                for (unsigned int v = 0; v < mesh->mNumVertices; ++v)
                {
                    const aiVector3D& pos = mesh->mVertices[v];
                    const aiVector3D& norm = mesh->mNormals[v];
                    const aiVector3D& tangent = mesh->mTangents[v];
                    const aiVector3D& texcoord = mesh->mTextureCoords[0][v];

                    model.vertices.emplace_back(glm::vec3(pos.x, pos.y, pos.z),
                                                glm::vec2(texcoord.x, texcoord.y),
                                                glm::vec3(norm.x, norm.y, norm.z),
                                                glm::vec3(tangent.x, tangent.y, tangent.z));
                }

                for (unsigned int f = 0; f < mesh->mNumFaces; ++f)
                {
                    const aiFace& face = mesh->mFaces[f];
                    for (unsigned int j = 0; j < face.mNumIndices; ++j)
                        model.indices.push_back(face.mIndices[j]);
                }

                if (scene->HasMaterials() && mesh->mMaterialIndex < scene->mNumMaterials)
                {
                    const aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
                    aiString          tex_path;
                    if (material->GetTexture(aiTextureType_DIFFUSE, 0, &tex_path) == AI_SUCCESS)
                    {
                        model.diffuse_path = tex_path.C_Str();
                        all_textures[model.diffuse_path] = aiTextureType_DIFFUSE;
                    }
                    if (material->GetTexture(aiTextureType_METALNESS, 0, &tex_path) == AI_SUCCESS)
                    {
                        model.metallic_path = tex_path.C_Str();
                        all_textures[model.metallic_path] = aiTextureType_METALNESS;
                    }
                    if (material->GetTexture(aiTextureType_NORMALS, 0, &tex_path) == AI_SUCCESS)
                    {
                        model.normal_path = tex_path.C_Str();
                        all_textures[model.normal_path] = aiTextureType_NORMALS;
                    }
                }

                generate_meshlet(model, mesh);

                out_scene.models.push_back(std::move(model));
            }

            for (unsigned int child_index = 0; child_index < node->mNumChildren; ++child_index)
            {
                process_node(node->mChildren[child_index], world_matrix);
            }
        };
        process_node(scene->mRootNode, aiMatrix4x4());

        {
            int          width{};
            int          height{};
            int          channels{};
            float* const pixels = stbi_loadf((path.parent_path() / "circus_arena_4k.hdr").string().c_str(), &width, &height, &channels, 4);
        }

        for (const std::pair<const std::string, aiTextureType>& texture : all_textures)
        {
            int      tex_width{}, tex_height{}, channels{};
            stbi_uc* pixels;
            if (texture.first[0] == '*')
            {
                int index = std::stoi(texture.first.substr(1));
                pixels = stbi_load_from_memory(reinterpret_cast<stbi_uc const*>(scene->mTextures[index]->pcData), scene->mTextures[index]->mWidth, &tex_width, &tex_height, &channels, STBI_rgb_alpha);
            }
            else
            {
                pixels = stbi_load((path.parent_path() / texture.first).string().c_str(), &tex_width, &tex_height, &channels, STBI_rgb_alpha);
            }
            channels = 4;

            if (!pixels)
            {
                spdlog::error("{}", stbi_failure_reason());
                throw std::runtime_error("failed to load texture image!");
            }

            out_scene.textures.emplace_back(
                texture.first,
                tex_width,
                tex_height,
                channels,
                texture.second,
                pixels);
        }

        aiReleaseImport(scene);

        // std::vector<meshopt_Meshlet> meshlets;
        // std::vector<uint32_t> meshlet_vertices;
        // std::vector<uint8_t> meshlet_triangles;

        // const aiScene* horse_testing = aiImportFile("resources/rossbandiger/scene.gltf", aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_CalcTangentSpace);

        // aiMesh* mesh = horse_testing->mMeshes[0];

        return out_scene;
    }

} // namespace

pvp::LoadedScene pvp::load_scene_cpu(const std::filesystem::path& path)
{
    LoadedScene scene;
    if (has_cache(path))
    {
        load_cache(path, scene);
    }
    else
    {
        scene = load_scene_from_disk(path);
        save_cache(path, scene);
    }

    return scene;
}
