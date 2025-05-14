#pragma once
#include <unordered_map>
#include <Context/Context.h>
#include <vulkan/vulkan.h>

namespace pvp
{
    class DescriptorLayoutBuilder;

    class DescriptorCreator final
    {
    public:
        explicit DescriptorCreator(const Context& context);
        DescriptorLayoutBuilder create_layout();
        void                    add_layout(uint32_t index, VkDescriptorSetLayout layout);

        VkDescriptorSetLayout get_layout(uint32_t index) const
        {
            return m_layouts.at(index);
        };

        VkDescriptorPool get_pool() const
        {
            return m_pool;
        };
        void destroy() const;

    private:
        const Context&   m_context;
        VkDescriptorPool m_pool{};

        std::unordered_map<uint32_t, VkDescriptorSetLayout> m_layouts;
    };
} // namespace pvp
