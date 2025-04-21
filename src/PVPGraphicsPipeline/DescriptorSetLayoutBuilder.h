#pragma once
#include <vulkan/vulkan.h>
namespace pvp
{
    class DescriptorSetLayoutBuilder
    {
        public:
        VkDescriptorSetLayout build(VkDevice device);
    };
} // namespace pvp
