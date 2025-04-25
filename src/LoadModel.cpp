#include "LoadModel.h"

#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
void LoadModel::load_file(const std::filesystem::path& path)
{
    const aiScene* scene = aiImportFile(path.generic_string().c_str(), aiProcess_Triangulate);

    verties.reserve(scene->mMeshes[0]->mNumVertices);
    for (unsigned int i = 0; i < scene->mMeshes[0]->mNumVertices; ++i)
    {
        const aiVector3D& vertex = scene->mMeshes[0]->mVertices[i];
        verties.emplace_back(glm::vec3(vertex.x, vertex.y, vertex.z), glm::vec2());
    }
}