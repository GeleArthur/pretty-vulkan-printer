#pragma once
#include <vulkan/vulkan.hpp>

#include <vector>
#include <Context/Context.h>
namespace pvp
{
    class DescriptorCreator;
    class DescriptorLayoutBuilder
    {
    public:
        DescriptorLayoutBuilder& add_binding(VkDescriptorType type, VkShaderStageFlags stage, uint32_t amount = 1u);
        DescriptorLayoutBuilder& add_flag(VkDescriptorBindingFlags flag);
        DescriptorCreator&       build(uint32_t index);

    private:
        friend class DescriptorCreator;
        explicit DescriptorLayoutBuilder(const Context& context, DescriptorCreator& creator);
        std::vector<VkDescriptorSetLayoutBinding> m_bindings;
        std::vector<VkDescriptorBindingFlags>     m_flags;
        DescriptorCreator&                        m_creator;
        const Context&                            m_context;
    };
} // namespace pvp
