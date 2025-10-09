#include "ModelData.h"

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

glm::mat4 convert_matrix(const aiMatrix4x4& m)
{
    glm::mat4 out;
    out[0] = glm::vec4(m.a1, m.b1, m.c1, m.d1);
    out[1] = glm::vec4(m.a2, m.b2, m.c2, m.d2);
    out[2] = glm::vec4(m.a3, m.b3, m.c3, m.d3);
    out[3] = glm::vec4(m.a4, m.b4, m.c4, m.d4);
    return out;
}

pvp::LoadedScene pvp::load_scene_cpu(const std::filesystem::path& path)
{
    LoadedScene    out_scene;
    const aiScene* scene = aiImportFile(path.generic_string().c_str(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_CalcTangentSpace);

    if (scene == nullptr)
    {
        spdlog::error("Failed to load model: {}", aiGetErrorString());
        return out_scene;
    }

    std::unordered_map<std::string, aiTextureType> all_textures;

    std::function<void(aiNode*)> process_node;
    process_node = [&](aiNode* node, const aiMatrix4x4& parent_transform = aiMatrix4x4()) {
        glm::mat4 node_transform = convert_matrix(parent_transform * node->mTransformation);

        for (unsigned int i = 0; i < node->mNumMeshes; ++i)
        {
            const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

            ModelData model;
            model.vertices.reserve(mesh->mNumVertices);
            model.indices.reserve(mesh->mNumFaces * 3);
            model.transform = node_transform;

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

            out_scene.models.push_back(std::move(model));
        }

        for (unsigned int c = 0; c < node->mNumChildren; ++c)
            process_node(node->mChildren[c]);
    };
    process_node(scene->mRootNode);

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

    return out_scene;
}
