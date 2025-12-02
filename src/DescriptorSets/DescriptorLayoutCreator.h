#pragma once
#include <unordered_map>
#include <Context/Context.h>
#include <vulkan/vulkan.h>

enum class DiscriptorTag;
namespace pvp
{
    class DescriptorLayoutBuilder;

    class DescriptorLayoutCreator final
    {
    public:
        explicit DescriptorLayoutCreator(const Context& context);
        ~DescriptorLayoutCreator();
        DescriptorLayoutBuilder get_layout();

        VkDescriptorPool get_pool() const
        {
            return m_pool;
        };

    private:
        friend class DescriptorLayoutBuilder;
        const Context&   m_context;
        VkDescriptorPool m_pool{};

        std::unordered_map<uint32_t, VkDescriptorSetLayout> m_layouts;
        std::unordered_map<DiscriptorTag, uint32_t>         m_tag_to_layout;
    };
} // namespace pvp
