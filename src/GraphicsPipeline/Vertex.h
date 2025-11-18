#pragma once
#include <array>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vulkan/vulkan.h>

namespace pvp
{
    struct Vertex
    {
        alignas(16) glm::vec3 pos;
        alignas(16) glm::vec2 uv;
        alignas(16) glm::vec3 normal;
        alignas(16) glm::vec3 tangent;

        static constexpr auto get_attribute_descriptions()
        {
            return std::vector<VkVertexInputAttributeDescription>(
                { {
                      .location = 0,
                      .binding = 0,
                      .format = VK_FORMAT_R32G32B32_SFLOAT,
                      .offset = offsetof(Vertex, pos),
                  },
                  {
                      .location = 1,
                      .binding = 0,
                      .format = VK_FORMAT_R32G32_SFLOAT,
                      .offset = offsetof(Vertex, uv),
                  },
                  {
                      .location = 2,
                      .binding = 0,
                      .format = VK_FORMAT_R32G32B32_SFLOAT,
                      .offset = offsetof(Vertex, normal),
                  },
                  {
                      .location = 3,
                      .binding = 0,
                      .format = VK_FORMAT_R32G32B32_SFLOAT,
                      .offset = offsetof(Vertex, tangent),
                  }

                });
        };
        static constexpr auto get_binding_description()
        {
            return std::vector<VkVertexInputBindingDescription>(
                { {
                    .binding = 0,
                    .stride = sizeof(Vertex),
                    .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
                } });
        }
    };
} // namespace pvp
