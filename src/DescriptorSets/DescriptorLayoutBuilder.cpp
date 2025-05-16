#include "DescriptorLayoutBuilder.h"

#include "DescriptorCreator.h"

#include <stdexcept>
#include <Context/Device.h>

namespace pvp
{
    DescriptorLayoutBuilder::DescriptorLayoutBuilder(const Context& context, DescriptorCreator& creator)
        : m_creator(creator)
        , m_context(context)
    {
    }
    DescriptorLayoutBuilder& DescriptorLayoutBuilder::add_binding(VkDescriptorType type, VkShaderStageFlags stage, uint32_t amount)
    {
        m_bindings.emplace_back(
            static_cast<uint32_t>(m_bindings.size()),
            type,
            amount,
            stage,
            VK_NULL_HANDLE);
        return *this;
    }
    DescriptorLayoutBuilder& DescriptorLayoutBuilder::add_flag(VkDescriptorBindingFlags flag)
    {
        m_flags.push_back(flag);
        return *this;
    }

    DescriptorCreator& DescriptorLayoutBuilder::build(uint32_t index)
    {
        VkDescriptorSetLayoutBindingFlagsCreateInfo extra_flags{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
            .bindingCount = static_cast<uint32_t>(m_flags.size()),
            .pBindingFlags = m_flags.data()
        };

        const VkDescriptorSetLayoutCreateInfo layout_info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = &extra_flags,
            .flags = 0,
            .bindingCount = static_cast<uint32_t>(m_bindings.size()),
            .pBindings = m_bindings.data()
        };

        VkDescriptorSetLayout result;

        if (vkCreateDescriptorSetLayout(m_context.device->get_device(), &layout_info, nullptr, &result) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }

        m_creator.add_layout(index, result);

        return m_creator;
    }
} // namespace pvp