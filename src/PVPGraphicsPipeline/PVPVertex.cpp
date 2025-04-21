#include "PVPVertex.h"

std::vector<VkVertexInputAttributeDescription> pvp::PvpVertex::get_attribute_descriptions()
{
    std::vector<VkVertexInputAttributeDescription> attribute_descriptions(2);

    attribute_descriptions[0].binding = 0;
    attribute_descriptions[0].location = 0;
    attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_descriptions[0].offset = offsetof(PvpVertex, pos);

    attribute_descriptions[1].binding = 0;
    attribute_descriptions[1].location = 1;
    attribute_descriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
    attribute_descriptions[1].offset = offsetof(PvpVertex, uv);

    return attribute_descriptions;
}
std::vector<VkVertexInputBindingDescription> pvp::PvpVertex::get_binding_description()
{
    std::vector<VkVertexInputBindingDescription> attribute_descriptions(1);
    attribute_descriptions[0].binding = 0;
    attribute_descriptions[0].stride = sizeof(PvpVertex);
    attribute_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return attribute_descriptions;
}
bool pvp::PvpVertex::operator==(PvpVertex const& other) const
{
    return pos == other.pos && uv == other.uv;
}