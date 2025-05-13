#pragma once
#include <vulkan/vulkan.hpp>

#include <vector>
namespace pvp
{
    class DescriptorLayoutBuilder
    {
    public:
        DescriptorLayoutBuilder& add_binding(VkDescriptorType type, VkShaderStageFlags stage);
        void                     build(const VkDevice& device, VkDescriptorSetLayout& layout);

    private:
        std::vector<VkDescriptorSetLayoutBinding> m_bindings;
    };
} // namespace pvp
