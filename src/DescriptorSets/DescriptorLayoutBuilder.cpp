#include "DescriptorLayoutBuilder.h"

#include "CommonDescriptorLayouts.h"
#include "DescriptorLayoutCreator.h"

#include <stdexcept>
#include <Context/Device.h>
#include <tracy/Tracy.hpp>

namespace pvp
{
    DescriptorLayoutBuilder::DescriptorLayoutBuilder(DescriptorLayoutCreator& creator)
        : m_creator{ creator }
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
    DescriptorLayoutBuilder& DescriptorLayoutBuilder::set_tag(DiscriptorTag tag)
    {
        m_set_tag = tag;
        return *this;
    }
    DescriptorLayoutBuilder& DescriptorLayoutBuilder::from_tag(DiscriptorTag tag)
    {
        m_from_tag = tag;
        return *this;
    }
    DescriptorLayoutBuilder& DescriptorLayoutBuilder::add_flag(VkDescriptorBindingFlags flag)
    {
        m_flags.push_back(flag);
        return *this;
    }
    VkDescriptorSetLayout DescriptorLayoutBuilder::get()
    {
        if (m_from_tag != DiscriptorTag::nothing)
        {
            return m_creator.m_layouts.at(m_creator.m_tag_to_layout.at(m_from_tag));
        }

        const size_t          hash = get_hash();
        VkDescriptorSetLayout result{};

        if (!m_creator.m_layouts.contains(hash))
        {
            ZoneScoped;
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

            if (vkCreateDescriptorSetLayout(m_creator.m_context.device->get_device(), &layout_info, nullptr, &result) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create descriptor set layout!");
            }

            m_creator.m_layouts[hash] = result;
        }
        else
        {
            result = m_creator.m_layouts.at(hash);
        }

        if (m_set_tag != DiscriptorTag::nothing)
        {
            m_creator.m_tag_to_layout[m_set_tag] = hash;
        }

        return result;
    }

    template<class T>
    void hash_combine(std::size_t& seed, const T& v)
    {
        std::hash<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    std::size_t DescriptorLayoutBuilder::get_hash() const
    {
        std::size_t thehash;
        for (const VkDescriptorSetLayoutBinding& layout : m_bindings)
        {
            hash_combine(thehash, layout.binding);
            hash_combine(thehash, layout.descriptorType);
            hash_combine(thehash, layout.descriptorCount);
            hash_combine(thehash, layout.stageFlags);
        }
        for (const VkDescriptorBindingFlags& flag : m_flags)
        {
            hash_combine(thehash, flag);
        }
        return thehash;
    }

    // DescriptorLayoutCreator& DescriptorLayoutBuilder::build(uint32_t index)
    // {
    //     ZoneScoped;
    //     VkDescriptorSetLayoutBindingFlagsCreateInfo extra_flags{
    //         .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
    //         .bindingCount = static_cast<uint32_t>(m_flags.size()),
    //         .pBindingFlags = m_flags.data()
    //     };
    //
    //     const VkDescriptorSetLayoutCreateInfo layout_info{
    //         .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
    //         .pNext = &extra_flags,
    //         .flags = 0,
    //         .bindingCount = static_cast<uint32_t>(m_bindings.size()),
    //         .pBindings = m_bindings.data()
    //     };
    //
    //     VkDescriptorSetLayout result;
    //
    //     if (vkCreateDescriptorSetLayout(m_context.device->get_device(), &layout_info, nullptr, &result) != VK_SUCCESS)
    //     {
    //         throw std::runtime_error("failed to create descriptor set layout!");
    //     }
    //
    //     m_creator.add_layout(index, result);
    //
    //     return m_creator;
    // }
} // namespace pvp