#pragma once
#include "../Renderer/FrameContext.h"
#include <vulkan/vulkan.h>
#include <array>
#include "../globalconst.h"

namespace pvp
{
    class DescriptorSets final
    {
    public:
        const VkDescriptorSet* get_descriptor_set(const FrameContext& context) const
        {
            return &m_sets[context.buffer_index];
        }

    private:
        friend class DescriptorSetBuilder;
        std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> m_sets{ VK_NULL_HANDLE };
    };
} // namespace pvp
