#include "DescriptorLayoutCreator.h"

#include "DescriptorLayoutBuilder.h"

#include <array>
#include <stdexcept>
#include <Context/Device.h>
#include <spdlog/spdlog.h>

pvp::DescriptorLayoutCreator::DescriptorLayoutCreator(const Context& context)
    : m_context{ context }
{
    const std::array sizes = {
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
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

pvp::DescriptorLayoutCreator::~DescriptorLayoutCreator()
{
    for (auto& layout : m_layouts)
    {
        vkDestroyDescriptorSetLayout(m_context.device->get_device(), layout.second, nullptr);
    }
    vkDestroyDescriptorPool(m_context.device->get_device(), m_pool, nullptr);
}

pvp::DescriptorLayoutBuilder pvp::DescriptorLayoutCreator::create_layout()
{
    return DescriptorLayoutBuilder{ m_context, *this };
}

void pvp::DescriptorLayoutCreator::add_layout(uint32_t index, VkDescriptorSetLayout layout)
{
    if (m_layouts.contains(index))
    {
        spdlog::warn("Overwriting descriptor set layout: {} ", index);
        vkDestroyDescriptorSetLayout(m_context.device->get_device(), m_layouts[index], nullptr);
    }

    m_layouts[index] = layout;
}
void pvp::DescriptorLayoutCreator::remove_layout(uint32_t index)
{
    vkDestroyDescriptorSetLayout(m_context.device->get_device(), m_layouts[index], nullptr);
    m_layouts.erase(index);
}
