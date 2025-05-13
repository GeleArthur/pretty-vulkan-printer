#pragma once
#include <filesystem>
#include <GraphicsPipeline/Vertex.h>

namespace pvp
{
    struct Vertex;
    struct LoadModel
    {
        std::vector<pvp::Vertex> verties;
        std::vector<uint32_t>    indices;
    };
    LoadModel load_model_file(const std::filesystem::path& path);
} // namespace pvp