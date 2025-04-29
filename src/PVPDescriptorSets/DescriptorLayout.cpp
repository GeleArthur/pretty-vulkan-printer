#include "DescriptorLayout.h"

#include <vector>

namespace pvp
{
    DescriptorLayout::DescriptorLayout(const VkDevice device, const std::vector<VkDescriptorSetLayoutBinding>& bindings)
        : m_device{ device }
    {
        VkDescriptorSetLayoutCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        create_info.bindingCount = bindings.size();
        create_info.pBindings = bindings.data();

        vkCreateDescriptorSetLayout(device, &create_info, nullptr, &m_layout);
    }
    DescriptorLayout::~DescriptorLayout()
    {
        destroy();
    }

    const VkDescriptorSetLayout& DescriptorLayout::get_handle()
    {
        return m_layout;
    }

    void DescriptorLayout::destroy() const
    {
        vkDestroyDescriptorSetLayout(m_device, m_layout, nullptr);
    }
} // namespace pvp
