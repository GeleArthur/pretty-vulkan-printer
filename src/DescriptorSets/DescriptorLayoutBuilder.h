#pragma once
#include <vulkan/vulkan.hpp>

#include <vector>
#include <Context/Context.h>
namespace pvp
{
    class DescriptorLayoutCreator;
    class DescriptorLayoutBuilder final
    {
    public:
        DescriptorLayoutBuilder& add_binding(VkDescriptorType type, VkShaderStageFlags stage, uint32_t amount = 1u);
        DescriptorLayoutBuilder& add_flag(VkDescriptorBindingFlags flag);
        DescriptorLayoutCreator& build(uint32_t index);

    private:
        friend class DescriptorLayoutCreator;
        explicit DescriptorLayoutBuilder(const Context& context, DescriptorLayoutCreator& creator);
        std::vector<VkDescriptorSetLayoutBinding> m_bindings;
        std::vector<VkDescriptorBindingFlags>     m_flags;
        DescriptorLayoutCreator&                  m_creator;
        const Context&                            m_context;
    };
} // namespace pvp
