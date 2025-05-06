#pragma once
#include <vector>
#include <PVPGraphicsPipeline/Vertex.h>

struct ModelData
{
    std::vector<pvp::Vertex> vertices;
    std::vector<uint32_t>    indices;
    MaterialInfo             material;
};
