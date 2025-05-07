#include "RenderToSwapchain.h"

#include <stdexcept>

namespace pvp
{
    void RenderToSwapchain::build_command_buffer()
    {
        
        
        VkCommandBuffer          cmd = m_cmds_graphics[m_double_buffer_frame];
        VkCommandBufferBeginInfo begin_info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        if (vkBeginCommandBuffer(cmd, &begin_info) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        // Color transition
        {
            VkImageSubresourceRange range{};
            range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            range.baseMipLevel = 0;
            range.levelCount = VK_REMAINING_MIP_LEVELS;
            range.baseArrayLayer = 0;
            range.layerCount = VK_REMAINING_ARRAY_LAYERS;

            // VK_PIPELINE_STAGE_2_NONE
            VkImageMemoryBarrier2 image_memory_barrier{
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .pNext = nullptr,
                .srcStageMask = VK_PIPELINE_STAGE_2_NONE,
                .srcAccessMask = VK_ACCESS_2_NONE,

                .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = m_swapchain->get_images()[m_double_buffer_frame],
                .subresourceRange = range
            };

            VkDependencyInfo dependency_info{
                .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                .dependencyFlags = 0,
                .memoryBarrierCount = 0,
                .pMemoryBarriers = VK_NULL_HANDLE,
                .bufferMemoryBarrierCount = 0,
                .pBufferMemoryBarriers = VK_NULL_HANDLE,
                .imageMemoryBarrierCount = 1,
                .pImageMemoryBarriers = &image_memory_barrier
            };

            vkCmdPipelineBarrier2(cmd, &dependency_info);
        }

        VkClearValue clear_values{};
        clear_values.color = { { 0.2f, 0.2f, 0.2f, 1.0f } };
        clear_values.depthStencil = { 1.0f, 0 };

        VkRenderingAttachmentInfo color_info{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = m_swapchain->get_views()[m_double_buffer_frame],
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = clear_values
        };

        VkRenderingInfo render_info{
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .renderArea = VkRect2D{ VkOffset2D{ 0, 0 }, m_swapchain->get_swapchain_extent() },
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &color_info,
        };

        vkCmdBeginRendering(cmd, &render_info);

        VkBlitImageInfo2 info{
            .sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
            // .srcImage = ,
            // .srcImageLayout =,
            // .dstImage =,
            // .dstImageLayout =,
            .regionCount = 1,
            // .pRegions =,
            .filter = VK_FILTER_LINEAR
        };

        vkCmdBlitImage2(cmd, &info);

        vkCmdEndRendering(cmd);

        // Present transition
        {
            VkImageSubresourceRange range{};
            range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            range.baseMipLevel = 0;
            range.levelCount = VK_REMAINING_MIP_LEVELS;
            range.baseArrayLayer = 0;
            range.layerCount = VK_REMAINING_ARRAY_LAYERS;

            // VK_PIPELINE_STAGE_2_NONE
            VkImageMemoryBarrier2 image_memory_barrier{
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .pNext = nullptr,
                .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_2_NONE,
                .dstAccessMask = VK_ACCESS_2_NONE,
                .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = m_swapchain->get_images()[m_double_buffer_frame],
                .subresourceRange = range
            };

            VkDependencyInfo dependency_info{
                .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                .dependencyFlags = 0,
                .memoryBarrierCount = 0,
                .pMemoryBarriers = VK_NULL_HANDLE,
                .bufferMemoryBarrierCount = 0,
                .pBufferMemoryBarriers = VK_NULL_HANDLE,
                .imageMemoryBarrierCount = 1,
                .pImageMemoryBarriers = &image_memory_barrier
            };

            vkCmdPipelineBarrier2(cmd, &dependency_info);
        }

        if (vkEndCommandBuffer(cmd) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to begin recording command buffer!");
        }
    }
} // namespace pvp