#pragma once
#include <PVPGraphicsPipeline/Vertex.h>
#include <filesystem>
#include <glm/vec3.hpp>

namespace pvp
{
    struct LoadModel
    {
        void load_file(const std::filesystem::path& path);

        std::vector<pvp::Vertex> verties;
        std::vector<uint32_t>    indices;
    };
} // namespace pvp