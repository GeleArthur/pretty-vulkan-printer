#pragma once
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vulkan/vulkan.h>

namespace pvp
{
    // :( formatting
    struct PvpVertex
    {
        glm::vec3                                             pos;
        glm::vec2                                             uv;

        static std::vector<VkVertexInputBindingDescription>   get_binding_description();
        static std::vector<VkVertexInputAttributeDescription> get_attribute_descriptions();

        bool                                                  operator==(PvpVertex const& other) const;
    };
} // namespace pvp
