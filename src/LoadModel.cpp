#include "LoadModel.h"

#include <iostream>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <spdlog/spdlog.h>
void LoadModel::load_file(const std::filesystem::path& path)
{
    const aiScene* scene = aiImportFile(path.generic_string().c_str(), aiProcess_Triangulate | aiProcess_FlipUVs);

    if (scene == nullptr)
    {
        spdlog::error("Failed to load model: {}", aiGetErrorString());
        return;
    }

    verties.reserve(scene->mMeshes[0]->mNumVertices);
    for (unsigned int i = 0; i < scene->mMeshes[0]->mNumVertices; ++i)
    {
        const aiVector3D& vertex = scene->mMeshes[0]->mVertices[i];
        const aiVector3D& uv = scene->mMeshes[0]->mTextureCoords[0][i];
        verties.emplace_back(glm::vec3(vertex.x, vertex.y, vertex.z), glm::vec2(uv.x, uv.y));
    }

    indices.reserve(scene->mMeshes[0]->mNumFaces * 3);
    for (unsigned int i = 0; i < scene->mMeshes[0]->mNumFaces; ++i)
        for (unsigned int j = 0; j < 3; ++j)
            indices.emplace_back(scene->mMeshes[0]->mFaces[i].mIndices[j]);

    aiReleaseImport(scene);
}