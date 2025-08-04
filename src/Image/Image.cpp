#include "Image.h"

#include "TransitionLayout.h"
#include "../Context/Context.h"

#include <VulkanExternalFunctions.h>
#include <Context/Device.h>
#include <Renderer/FrameContext.h>
#include <VMAAllocator/VmaAllocator.h>
#include <vulkan/vulkan.h>

namespace pvp
{
    VkImageView Image::get_view(const FrameContext& frame_context) const
    {
        return m_view[frame_context.buffer_index];
    }

    VkImageView Image::get_view(int index) const
    {
        return m_view[index];
    }

    VkImage Image::get_image(const FrameContext& frame_context) const
    {
        return m_image[frame_context.buffer_index];
    }
    VkImage Image::get_image(int index) const
    {
        return m_image[index];
    }

    const VmaAllocationInfo& Image::get_allocation_info() const
    {
        return m_allocation_info;
    }

    VkImageLayout Image::get_layout(const FrameContext& frame_context) const
    {
        return m_current_layout[frame_context.buffer_index];
    }
    VkImageLayout Image::get_layout(int index) const
    {
        return m_current_layout[index];
    }
    VkFormat Image::get_format() const
    {
        return m_create_info.format;
    }
    VkExtent2D Image::get_size() const
    {
        return VkExtent2D{ m_create_info.extent.width, m_create_info.extent.height };
    }
    Event<>& Image::get_image_invalid()
    {
        return m_image_invalid;
    }

    void Image::transition_layout(const FrameContext&   frame_context,
                                  VkImageLayout         new_layout,
                                  VkPipelineStageFlags2 src_stage_mask,
                                  VkPipelineStageFlags2 dst_stage_mask,
                                  VkAccessFlags2        src_access_mask,
                                  VkAccessFlags2        dst_access_mask)
    {
        VkImageSubresourceRange range{
            .aspectMask = m_view_create_info.subresourceRange.aspectMask,
            .baseMipLevel = 0,
            .levelCount = VK_REMAINING_MIP_LEVELS,
            .baseArrayLayer = 0,
            .layerCount = VK_REMAINING_ARRAY_LAYERS
        };
        image_layout_transition(frame_context.command_buffer,
                                m_image[frame_context.buffer_index],
                                src_stage_mask,
                                dst_stage_mask,
                                src_access_mask,
                                dst_access_mask,
                                m_current_layout[frame_context.buffer_index],
                                new_layout,
                                range);
        m_current_layout[frame_context.buffer_index] = new_layout;
    }

    void Image::create_images(const Context& context)
    {
        for (int i = 0; i < max_frames_in_flight; ++i)
        {
            if (vmaCreateImage(context.allocator->get_allocator(), &m_create_info, &m_allocation_create_info, &m_image[i], &m_allocation[i], &m_allocation_info) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed creating image");
            }
            m_current_layout[i] = VK_IMAGE_LAYOUT_UNDEFINED;

            m_view_create_info.image = m_image[i];

            if (vkCreateImageView(context.device->get_device(), &m_view_create_info, nullptr, &m_view[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed creating image view");
            }
#if defined(_DEBUG)

            VkDebugUtilsObjectNameInfoEXT image_debug{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .objectType = VK_OBJECT_TYPE_IMAGE,
                .objectHandle = reinterpret_cast<uint64_t>(m_image[i]),
                .pObjectName = m_name.c_str()
            };

            VulkanInstanceExtensions::vkSetDebugUtilsObjectNameEXT(context.device->get_device(), &image_debug);
#endif
        }
    }

    void Image::destroy(const Context& context) const
    {
        for (int i = 0; i < max_frames_in_flight; ++i)
        {
            vmaDestroyImage(context.allocator->get_allocator(), m_image[i], m_allocation[i]);
            vkDestroyImageView(context.device->get_device(), m_view[i], nullptr);
        }
    }

    void Image::resize_image(const Context& context, int width, int height)
    {
        destroy(context);

        m_create_info.extent.width = static_cast<uint32_t>(width);
        m_create_info.extent.height = static_cast<uint32_t>(height);

        create_images(context);

        m_image_invalid.notify_listeners();
    }
} // namespace pvp
