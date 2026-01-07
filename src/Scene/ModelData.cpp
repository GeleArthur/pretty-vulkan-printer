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
#include <tracy/Tracy.hpp>
#include <dds.hpp>
#include <PodHelpers.h>

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

    void writeOBJ(const std::vector<glm::vec4>& points, const std::string& filename)
    {
        std::ofstream file(filename);

        if (!file.is_open())
        {
            std::cerr << "Error: Could not open file " << filename << std::endl;
            return;
        }

        // Write header comment
        file << "# OBJ file with " << points.size() << " vertices\n";
        // Write vertices
        for (int i = 0; i < points.size(); ++i)
        {
            const glm::vec4& point = points[i];
            file << "v " << point.x << " " << point.y << " " << point.z << " " << point.w << " " << static_cast<float>(i) << " " << "0.0" << "\n";
        }
        for (const auto& point : points)
        {
        }

        file.close();
        std::cout << "Successfully wrote " << points.size() << " vertices to " << filename << std::endl;
    }

    void writeOBJMeshLets(const std::string&                  filename,
                          const std::vector<meshopt_Meshlet>& meshlets,
                          const std::vector<uint32_t>&        meshlet_vertices,
                          const std::vector<uint8_t>&         meshlet_triangles,
                          const std::vector<float>&           vertices,
                          size_t                              meshlet_count)
    {
        std::ofstream file(filename);
        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open file: " + filename);
        }

        for (size_t i = 0; i < vertices.size(); i += 3)
        {
            file << "v " << vertices[i] << " " << vertices[i + 1] << " " << vertices[i + 2] << "\n";
        }

        for (size_t m = 0; m < meshlet_count; m++)
        {
            const auto& meshlet = meshlets[m];

            file << "# Meshlet " << m << "\n";
            file << "g meshlet_" << m << "\n";

            // Write faces for this meshlet
            for (uint32_t t = 0; t < meshlet.triangle_count; t++)
            {
                uint32_t tri_offset = meshlet.triangle_offset + t * 3;

                uint8_t v0 = meshlet_triangles[tri_offset + 0];
                uint8_t v1 = meshlet_triangles[tri_offset + 1];
                uint8_t v2 = meshlet_triangles[tri_offset + 2];

                uint32_t global_v0 = meshlet_vertices[meshlet.vertex_offset + v0] + 1;
                uint32_t global_v1 = meshlet_vertices[meshlet.vertex_offset + v1] + 1;
                uint32_t global_v2 = meshlet_vertices[meshlet.vertex_offset + v2] + 1;

                file << "f " << global_v0 << " " << global_v1 << " " << global_v2 << "\n";
            }
        }
        file.close();
    }

    void generate_meshlet(pvp::ModelData& model_out)
    {
        constexpr size_t max_vertices = 64;
        constexpr size_t max_triangles = 126;
        constexpr float  cone_weight = 0.25f;

        // std::vector<uint8_t> meshlet_triangles_u8;

        const size_t max_mesh_lets = meshopt_buildMeshletsBound(model_out.indices.size(), max_vertices, max_triangles);
        model_out.meshlets.resize(max_mesh_lets);
        model_out.meshlet_vertices.resize(max_mesh_lets * max_vertices);
        model_out.meshlet_triangles.resize(max_mesh_lets * max_triangles * 3);

        std::vector<float> vertices;
        vertices.reserve(model_out.vertices.size() * 3);
        for (pvp::Vertex& vertex : model_out.vertices)
        {
            vertices.push_back(vertex.pos.x);
            vertices.push_back(vertex.pos.y);
            vertices.push_back(vertex.pos.z);
        }

        const size_t meshlet_count = meshopt_buildMeshlets(
            model_out.meshlets.data(),
            model_out.meshlet_vertices.data(),
            model_out.meshlet_triangles.data(),
            model_out.indices.data(),
            model_out.indices.size(),
            vertices.data(),
            vertices.size(),
            sizeof(float) * 3,
            max_vertices,
            max_triangles,
            cone_weight);

        const meshopt_Meshlet& last = model_out.meshlets[meshlet_count - 1];
        model_out.meshlet_vertices.resize(last.vertex_offset + last.vertex_count);
        model_out.meshlet_triangles.resize(last.triangle_offset + last.triangle_count * 3);
        model_out.meshlets.resize(meshlet_count);

        for (const meshopt_Meshlet& meshlet : model_out.meshlets)
        {
            meshopt_optimizeMeshlet(&model_out.meshlet_vertices[meshlet.vertex_offset], &model_out.meshlet_triangles[meshlet.triangle_offset], meshlet.triangle_count, meshlet.vertex_count);
        }

        // writeOBJMeshLets("Meshlets.obj", model_out.meshlets, model_out.meshlet_vertices, meshlet_triangles_u8, vertices, meshlet_count);

        // for (auto& meshlet : model_out.meshlets)
        // {
        // Save triangle offset for current meshlet
        // uint32_t triangle_offset = static_cast<uint32_t>(model_out.meshlet_triangles.size());

        // Repack to uint32_t
        // for (uint32_t i = 0; i < meshlet.triangle_count; ++i)
        // {
        // uint32_t i0 = 3 * i + 0 + meshlet.triangle_offset;
        // uint32_t i1 = 3 * i + 1 + meshlet.triangle_offset;
        // uint32_t i2 = 3 * i + 2 + meshlet.triangle_offset;
        //
        // uint8_t  vertex_id0 = meshlet_triangles_u8[i0];
        // uint8_t  vertex_id1 = meshlet_triangles_u8[i1];
        // uint8_t  vertex_id2 = meshlet_triangles_u8[i2];
        // uint32_t packed = ((static_cast<uint32_t>(vertex_id0) & 0xFF) << 0) |
        //     ((static_cast<uint32_t>(vertex_id1) & 0xFF) << 8) |
        //     ((static_cast<uint32_t>(vertex_id2) & 0xFF) << 16);
        // model_out.meshlet_triangles.push_back(packed);
        // model_out.meshlet_triangles
        // }

        // Update triangle offset for current meshlet
        // meshlet.triangle_offset = triangle_offset;
        // }

        model_out.meshlet_sphere_bounds.reserve(model_out.meshlets.size());
        for (const auto& meshlet : model_out.meshlets)
        {
            meshopt_Bounds bounds = meshopt_computeMeshletBounds(
                &model_out.meshlet_vertices[meshlet.vertex_offset],
                &model_out.meshlet_triangles[meshlet.triangle_offset],
                meshlet.triangle_count,
                vertices.data(),
                vertices.size(),
                sizeof(float) * 3);
            model_out.meshlet_sphere_bounds.emplace_back(
                glm::vec4(bounds.center[0], bounds.center[1], bounds.center[2], bounds.radius),
                glm::vec4(bounds.cone_axis[0],
                          bounds.cone_axis[1],
                          bounds.cone_axis[2],
                          bounds.cone_cutoff));
        }

        // writeOBJ(model_out.meshlet_sphere_bounds, "OutPounts.obj");
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
            write_pod(out_stream, texture.format);
            write_pod(out_stream, texture.generate_mip_maps);
            write_vector(out_stream, texture.pixels);
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
            read_pod(in_stream, texture.format);
            read_pod(in_stream, texture.generate_mip_maps);
            read_vector(in_stream, texture.pixels);
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

    std::optional<pvp::LoadedScene> load_scene_from_disk(const std::filesystem::path& path)
    {
        pvp::LoadedScene out_scene{};
        const aiScene*   scene = aiImportFile(path.generic_string().c_str(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_CalcTangentSpace);

        if (scene == nullptr)
        {
            spdlog::error("Failed to load model: {}", aiGetErrorString());
            return {};
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

                generate_meshlet(model);

                out_scene.models.push_back(std::move(model));
            }

            for (unsigned int child_index = 0; child_index < node->mNumChildren; ++child_index)
            {
                process_node(node->mChildren[child_index], world_matrix);
            }
        };
        process_node(scene->mRootNode, aiMatrix4x4());

        // Cubemap loading later
        {
            int width{};
            int height{};
            int channels{};
            // float* const pixels = stbi_loadf((path.parent_path() / "circus_arena_4k.hdr").string().c_str(), &width, &height, &channels, 4);
        }

        for (const auto& [texture_name, texture_type] : all_textures)
        {
            pvp::TextureData loaded_texture{};

            if (const aiTexture* data = scene->GetEmbeddedTexture(texture_name.c_str()); data != nullptr)
            {
                int const index = std::stoi(texture_name.substr(1));
                int       tex_width{};
                int       tex_height{};
                int       channels{};

                uint8_t* pixels = stbi_load_from_memory(reinterpret_cast<stbi_uc const*>(scene->mTextures[index]->pcData),
                                                        static_cast<int>(scene->mTextures[index]->mWidth),
                                                        &tex_width,
                                                        &tex_height,
                                                        &channels,
                                                        STBI_rgb_alpha);

                if (!pixels)
                {
                    spdlog::error("{}", stbi_failure_reason());
                    throw std::runtime_error("failed to load texture image!");
                }

                const auto pixel_span = std::span(pixels, tex_height * tex_width * 4);

                loaded_texture.width = tex_width;
                loaded_texture.height = tex_height;
                loaded_texture.pixels = std::vector(pixel_span.begin(), pixel_span.end());
                loaded_texture.generate_mip_maps = true;
                loaded_texture.name = texture_name;
                switch (texture_type)
                {
                    case aiTextureType_DIFFUSE:
                        loaded_texture.format = VK_FORMAT_R8G8B8A8_SRGB;
                        break;
                    default:
                        loaded_texture.format = VK_FORMAT_R8G8B8A8_UNORM;
                }
            }
            else // Not embeded loading from disk
            {
                if (std::filesystem::path(texture_name).extension() == ".dds")
                {
                    dds::Image image;
                    if (dds::readFile((path.parent_path() / texture_name).string(), &image) != dds::Success)
                    {
                        spdlog::error("Can't load dds texture {}", texture_name);
                    }

                    loaded_texture.format = dds::getVulkanFormat(image.format, true);
                    loaded_texture.pixels = std::vector(image.mipmaps[0].cbegin(), image.mipmaps[0].cend());
                    loaded_texture.width = image.width;
                    loaded_texture.height = image.height;
                    loaded_texture.generate_mip_maps = false;
                    loaded_texture.name = texture_name;
                }
                else
                {
                    int      tex_width{};
                    int      tex_height{};
                    int      channels{};
                    uint8_t* pixels = stbi_load((path.parent_path() / texture_name).string().c_str(), &tex_width, &tex_height, &channels, STBI_rgb_alpha);

                    if (!pixels)
                    {
                        spdlog::error("{}", stbi_failure_reason());
                        throw std::runtime_error("failed to load texture image!");
                    }

                    const auto pixel_span = std::span(pixels, tex_height * tex_width * 4);
                    loaded_texture.width = tex_width;
                    loaded_texture.height = tex_height;
                    loaded_texture.pixels = std::vector(pixel_span.begin(), pixel_span.end());
                    loaded_texture.generate_mip_maps = true;
                    loaded_texture.name = texture_name;
                    switch (texture_type)
                    {
                        case aiTextureType_DIFFUSE:
                            loaded_texture.format = VK_FORMAT_R8G8B8A8_SRGB;
                            break;
                        default:
                            loaded_texture.format = VK_FORMAT_R8G8B8A8_UNORM;
                    }
                }
            }

            out_scene.textures.push_back(loaded_texture);
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

std::optional<pvp::LoadedScene> pvp::load_scene_cpu(const std::filesystem::path& path)
{
    ZoneScoped;
    if (!std::filesystem::exists(path))
    {
        return {};
    }
    LoadedScene scene;
    if (has_cache(path))
    {
        load_cache(path, scene);
    }
    else
    {
        std::optional<pvp::LoadedScene> maybe_scene = load_scene_from_disk(path);
        if (maybe_scene.has_value())
        {
            scene = maybe_scene.value();
            save_cache(path, scene);
        }
        else
        {
            return {};
        }
    }

    return scene;
}
