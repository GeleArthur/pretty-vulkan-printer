#include "DescriptorPool.h"

#include <stdexcept>

pvp::DescriptorPool::DescriptorPool(const VkDevice device, const std::vector<VkDescriptorPoolSize>& sizes, const uint32_t max_sets)
    : m_device{ device }
{
    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = static_cast<uint32_t>(sizes.size());
    pool_info.pPoolSizes = sizes.data();
    pool_info.maxSets = max_sets;

    if (vkCreateDescriptorPool(m_device, &pool_info, nullptr, &m_pool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void pvp::DescriptorPool::destroy() const
{
    vkDestroyDescriptorPool(m_device, m_pool, nullptr);
}
