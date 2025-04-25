#pragma once
#include <filesystem>
#include <PVPGraphicsPipeline/PVPVertex.h>
#include <glm/vec3.hpp>

struct LoadModel
{
    void                        load_file(const std::filesystem::path& path);

    std::vector<pvp::PvpVertex> verties;
};
