#include "DescriptorLayoutBuilder.h"
#include <stdexcept>

namespace pvp
{
    DescriptorLayoutBuilder& DescriptorLayoutBuilder::add_binding(VkDescriptorType type, VkShaderStageFlags stage)
    {
        m_bindings.push_back(VkDescriptorSetLayoutBinding{ static_cast<uint32_t>(m_bindings.size()), type, 1u, stage, VK_NULL_HANDLE });
        return *this;
    }

    void DescriptorLayoutBuilder::build(const VkDevice& device, VkDescriptorSetLayout& layout)
    {
        const VkDescriptorSetLayoutCreateInfo layout_info{
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            nullptr,
            0,
            static_cast<uint32_t>(m_bindings.size()),
            m_bindings.data()
        };

        if (vkCreateDescriptorSetLayout(device, &layout_info, nullptr, &layout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }
} // namespace pvp