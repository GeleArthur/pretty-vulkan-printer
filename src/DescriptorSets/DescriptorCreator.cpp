#include "DescriptorCreator.h"

#include "DescriptorLayoutBuilder.h"

#include <array>
#include <stdexcept>
#include <Context/Device.h>

pvp::DescriptorCreator::DescriptorCreator(const Context& context)
    : m_context{ context }
{
    const std::array sizes = {
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
    };

    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = static_cast<uint32_t>(sizes.size());
    pool_info.pPoolSizes = sizes.data();
    pool_info.maxSets = 1000;

    if (vkCreateDescriptorPool(context.device->get_device(), &pool_info, nullptr, &m_pool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

pvp::DescriptorCreator::~DescriptorCreator()
{
    for (auto& layout : m_layouts)
    {
        vkDestroyDescriptorSetLayout(m_context.device->get_device(), layout.second, nullptr);
    }
    vkDestroyDescriptorPool(m_context.device->get_device(), m_pool, nullptr);
}

pvp::DescriptorLayoutBuilder pvp::DescriptorCreator::create_layout()
{
    return DescriptorLayoutBuilder{ m_context, *this };
}

void pvp::DescriptorCreator::add_layout(uint32_t index, VkDescriptorSetLayout layout)
{
    if (m_layouts.contains(index))
    {
        vkDestroyDescriptorSetLayout(m_context.device->get_device(), m_layouts[index], nullptr);
    }

    m_layouts[index] = layout;
}
