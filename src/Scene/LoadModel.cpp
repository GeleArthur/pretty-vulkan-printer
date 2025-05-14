#include "LoadModel.h"

#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/fwd.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
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

std::vector<pvp::LoadModel> pvp::load_model_file(const std::filesystem::path& path)
{
    std::vector<LoadModel> out_models;
    const aiScene*         scene = aiImportFile(path.generic_string().c_str(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

    if (scene == nullptr)
    {
        spdlog::error("Failed to load model: {}", aiGetErrorString());
        return out_models;
    }

    std::function<void(aiNode*)> process_node;
    process_node = [&](aiNode* node, const aiMatrix4x4& parent_transform = aiMatrix4x4()) {
        glm::mat4 node_transform = convert_matrix(parent_transform * node->mTransformation);

        for (unsigned int i = 0; i < node->mNumMeshes; ++i)
        {
            const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

            LoadModel model;
            model.vertices.reserve(mesh->mNumVertices);
            model.indices.reserve(mesh->mNumFaces * 3);
            model.transform = node_transform; // Store transform *separately*

            for (unsigned int v = 0; v < mesh->mNumVertices; ++v)
            {
                const aiVector3D& pos = mesh->mVertices[v];
                const aiVector3D& norm = mesh->mNormals[v];
                aiVector3D        texcoord(0.0f, 0.0f, 0.0f);

                if (mesh->HasTextureCoords(0))
                    texcoord = mesh->mTextureCoords[0][v];

                model.vertices.emplace_back(glm::vec3(pos.x, pos.y, pos.z),
                                            glm::vec2(texcoord.x, texcoord.y),
                                            glm::vec3(norm.x, norm.y, norm.z));
            }

            for (unsigned int f = 0; f < mesh->mNumFaces; ++f)
            {
                const aiFace& face = mesh->mFaces[f];
                for (unsigned int j = 0; j < face.mNumIndices; ++j)
                    model.indices.push_back(face.mIndices[j]);
            }

            // if (scene->HasMaterials() && mesh->mMaterialIndex < scene->mNumMaterials)
            // {
            //     aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            //     aiString    tex_path;
            //     if (material->GetTexture(aiTextureType_DIFFUSE, 0, &tex_path) == AI_SUCCESS)
            //     {
            //         model.material.diffuse_texture = tex_path.C_Str();
            //     }
            // }

            out_models.push_back(std::move(model));
        }

        for (unsigned int c = 0; c < node->mNumChildren; ++c)
            process_node(node->mChildren[c]);
    };

    process_node(scene->mRootNode);

    aiReleaseImport(scene);

    return out_models;
}
