#pragma once
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
namespace pvp
{
    struct DebugVertex
    {
        alignas(16) glm::vec3 position;
        alignas(16) glm::vec4 color;

        static constexpr auto get_attribute_descriptions()
        {
            return std::vector<VkVertexInputAttributeDescription>(
                {
                    {
                        .location = 0,
                        .binding = 0,
                        .format = VK_FORMAT_R32G32B32_SFLOAT,
                        .offset = offsetof(DebugVertex, position),
                    },
                    {
                        .location = 1,
                        .binding = 0,
                        .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                        .offset = offsetof(DebugVertex, color),
                    },
                });
        };
        static constexpr auto get_binding_description()
        {
            return std::vector<VkVertexInputBindingDescription>(
                { {
                    .binding = 0,
                    .stride = sizeof(DebugVertex),
                    .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
                } });
        }
    };
} // namespace pvp
