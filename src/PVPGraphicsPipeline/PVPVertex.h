#pragma once
#include <array>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vulkan/vulkan.h>

namespace pvp
{
    // :( formatting
    struct PvpVertex
    {
        glm::vec3             pos;
        glm::vec2             uv;

        static constexpr auto get_attribute_descriptions()
        {
            return std::vector<VkVertexInputAttributeDescription>(
            { {
              .location = 0,
              .binding = 0,
              .format = VK_FORMAT_R32G32B32_SFLOAT,
              .offset = offsetof(PvpVertex, pos),
              },
              {
              .location = 1,
              .binding = 0,
              .format = VK_FORMAT_R32G32_SFLOAT,
              .offset = offsetof(PvpVertex, uv),
              } });
        };
        static constexpr auto get_binding_description()
        {
            return std::vector<VkVertexInputBindingDescription>(
            { {
            .binding = 0,
            .stride = sizeof(PvpVertex),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
            } });
        }
    };
} // namespace pvp
