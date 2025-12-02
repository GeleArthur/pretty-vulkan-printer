#pragma once
#include "CommonDescriptorLayouts.h"

#include <globalconst.h>
#include <iostream>
#include <vulkan/vulkan.hpp>

#include <vector>
#include <Context/Context.h>

enum class DiscriptorTag;
namespace pvp
{
    class DescriptorLayoutBuilder final
    {
    public:
        explicit DescriptorLayoutBuilder(DescriptorLayoutCreator& creator);
        DISABLE_COPY(DescriptorLayoutBuilder);
        DISABLE_MOVE(DescriptorLayoutBuilder);
        DescriptorLayoutBuilder& add_binding(VkDescriptorType type, VkShaderStageFlags stage, uint32_t amount = 1u);
        DescriptorLayoutBuilder& set_tag(DiscriptorTag tag);
        DescriptorLayoutBuilder& from_tag(DiscriptorTag tag);
        DescriptorLayoutBuilder& add_flag(VkDescriptorBindingFlags flag);
        VkDescriptorSetLayout    get();

    private:
        DescriptorLayoutCreator&                  m_creator;
        std::vector<VkDescriptorSetLayoutBinding> m_bindings;
        std::vector<VkDescriptorBindingFlags>     m_flags;
        DiscriptorTag                             m_from_tag{ DiscriptorTag::nothing };
        DiscriptorTag                             m_set_tag{ DiscriptorTag::nothing };
        std::size_t                               get_hash() const;
    };
} // namespace pvp
