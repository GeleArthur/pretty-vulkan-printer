#pragma once

#include <span>
#include <vector>
#include <vulkan/vulkan.h>

namespace pvp
{
    class DescriptorLayout final
    {
    public:
        DescriptorLayout(VkDevice device, const std::vector<VkDescriptorSetLayoutBinding>& bindings);
        ~DescriptorLayout();
        const VkDescriptorSetLayout& get_handle();
        void                         destroy() const;

    private:
        VkDevice              m_device;
        VkDescriptorSetLayout m_layout;
    };
} // namespace pvp
