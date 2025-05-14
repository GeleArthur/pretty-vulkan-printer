#pragma once
#include <filesystem>
#include <GraphicsPipeline/Vertex.h>
#include <glm/mat4x4.hpp>

namespace pvp
{
    struct Vertex;
    struct LoadModel
    {
        std::vector<Vertex>   vertices;
        std::vector<uint32_t> indices;
        glm::mat4x4           transform;
    };
    std::vector<LoadModel> load_model_file(const std::filesystem::path& path);
} // namespace pvp