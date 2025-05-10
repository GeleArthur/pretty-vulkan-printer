#pragma once

#include <span>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace pvp
{
    class DescriptorLayout final
    {
    public:
        explicit DescriptorLayout() = default;
        const vk::DescriptorSetLayout& get_handle() const;

        void destroy(const vk::Device& device) const;

    private:
        friend class DescriptorLayoutBuilder;
        vk::DescriptorSetLayout m_layout{};
    };
} // namespace pvp
