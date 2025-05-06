#pragma once
#include <vector>
#include <vulkan/vulkan.h>

namespace pvp
{
    class DescriptorPool final
    {
    public:
        explicit DescriptorPool() = default;
        explicit DescriptorPool(VkDevice device, const std::vector<VkDescriptorPoolSize>& sizes, uint32_t max_sets);
        const VkDescriptorPool& get_handle() const
        {
            return m_pool;
        };
        void destroy() const;

    private:
        VkDevice         m_device{};
        VkDescriptorPool m_pool{};
    };
} // namespace pvp
