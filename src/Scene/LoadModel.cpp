#include "LoadModel.h"

#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <spdlog/spdlog.h>

pvp::LoadModel pvp::load_model_file(const std::filesystem::path& path)
{
    LoadModel      model;
    const aiScene* scene = aiImportFile(path.generic_string().c_str(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

    if (scene == nullptr)
    {
        spdlog::error("Failed to load model: {}", aiGetErrorString());
        return model;
    }

    model.verties.reserve(scene->mMeshes[0]->mNumVertices);
    for (unsigned int i = 0; i < scene->mMeshes[0]->mNumVertices; ++i)
    {
        const aiVector3D& vertex = scene->mMeshes[0]->mVertices[i];
        const aiVector3D& uv = scene->mMeshes[0]->mTextureCoords[0][i];
        const aiVector3D& normal = scene->mMeshes[0]->mNormals[i];
        model.verties.emplace_back(glm::vec3(vertex.x, vertex.y, vertex.z),
                                   glm::vec2(uv.x, uv.y),
                                   glm::vec3(normal.x, normal.y, normal.z));
    }

    model.indices.reserve(scene->mMeshes[0]->mNumFaces * 3);
    for (unsigned int i = 0; i < scene->mMeshes[0]->mNumFaces; ++i)
        for (unsigned int j = 0; j < 3; ++j)
            model.indices.emplace_back(scene->mMeshes[0]->mFaces[i].mIndices[j]);

    aiReleaseImport(scene);

    return model;
}
