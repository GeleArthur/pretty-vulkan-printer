#include "BlitToSwapchain.h"

#include "FrameContext.h"
#include "RenderInfoBuilder.h"

#include <Context/Device.h>
#include <Debugger/Debugger.h>
#include <Image/ImageBuilder.h>
#include <Image/TransitionLayout.h>
#include <tracy/Tracy.hpp>
#include <tracy/TracyVulkan.hpp>

namespace pvp
{
    BlitToSwapchain::BlitToSwapchain(const Context& context, Image& source)
        : m_source_image{ source }
        , m_context{ context }
    {
    }

    void BlitToSwapchain::draw(const FrameContext& cmd, uint32_t swapchain_image_index)
    {
        ZoneScoped;
        TracyVkZone(m_context.tracy_ctx[cmd.buffer_index], cmd.command_buffer, "Blit To SwapChain");
        Debugger::start_debug_label(cmd.command_buffer, "Bilt to swapchain", { 0.5f, 0.5f, 0.0f });

        VkImageBlit2 region{
            .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
            .srcSubresource = VkImageSubresourceLayers{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1 },
            .srcOffsets = { VkOffset3D{ 0, 0, 0 }, VkOffset3D{ static_cast<int32_t>(m_source_image.get_size().width), static_cast<int32_t>(m_source_image.get_size().height), 1 } },
            .dstSubresource = VkImageSubresourceLayers{ .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1 },
            .dstOffsets = { VkOffset3D{ 0, 0, 0 }, VkOffset3D{ static_cast<int32_t>(m_context.swapchain->get_swapchain_extent().width), static_cast<int32_t>(m_context.swapchain->get_swapchain_extent().height), 1 } },
        };

        const VkBlitImageInfo2 info{
            .sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
            .srcImage = m_source_image.get_image(cmd),
            .srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            .dstImage = m_context.swapchain->get_images()[swapchain_image_index],
            .dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .regionCount = 1,
            .pRegions = &region,
            .filter = VK_FILTER_NEAREST
        };

        VkImageSubresourceRange range{
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = VK_REMAINING_MIP_LEVELS,
            .baseArrayLayer = 0,
            .layerCount = VK_REMAINING_ARRAY_LAYERS
        };

        image_layout_transition(cmd.command_buffer,
                                m_context.swapchain->get_images()[swapchain_image_index],
                                VK_PIPELINE_STAGE_2_NONE,
                                VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                                VK_ACCESS_2_NONE,
                                VK_ACCESS_2_TRANSFER_WRITE_BIT,
                                VK_IMAGE_LAYOUT_UNDEFINED,
                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                range);

        vkCmdBlitImage2(cmd.command_buffer, &info);
        image_layout_transition(cmd.command_buffer,
                                m_context.swapchain->get_images()[swapchain_image_index],
                                VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                                VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                VK_ACCESS_2_TRANSFER_WRITE_BIT,
                                VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT,
                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                range);
        Debugger::end_debug_label(cmd.command_buffer);
    }
} // namespace pvp
