#pragma once
#include "../Renderer/FrameContext.h"
#include <vulkan/vulkan.h>
#include <array>
#include "../globalconst.h"
#include <Image/Image.h>
#include <vector>

namespace pvp
{
    struct ImageBinding
    {
        uint32_t      binding;
        VkImageLayout layout;
        const Image*  image;
        int           set;
    };

    class DescriptorSets final
    {
    public:
        const VkDescriptorSet* get_descriptor_set(const FrameContext& context) const
        {
            return &m_sets[context.buffer_index];
        }

    private:
        friend class DescriptorSetBuilder;
        const Context* m_context;

        std::array<VkDescriptorSet, max_frames_in_flight> m_sets{ VK_NULL_HANDLE };
        std::vector<EventListener<>>                      m_images;

        void reconnect_image(const ImageBinding& binding);
    };
} // namespace pvp
