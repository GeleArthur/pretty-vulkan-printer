#include "TransitionLayout.h"

#include "../../cmake-build-x64-release/_deps/spdlog-src/include/spdlog/spdlog.h"

namespace pvp
{
    void image_layout_transition(
        VkCommandBuffer                command_buffer,
        VkImage                        image,
        VkPipelineStageFlags2          src_stage_mask,
        VkPipelineStageFlags2          dst_stage_mask,
        VkAccessFlags2                 src_access_mask,
        VkAccessFlags2                 dst_access_mask,
        VkImageLayout                  old_layout,
        VkImageLayout                  new_layout,
        VkImageSubresourceRange const& subresource_range)
    {
        VkImageMemoryBarrier2 image_memory_barrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .pNext = nullptr,
            .srcStageMask = src_stage_mask,
            .srcAccessMask = src_access_mask,
            .dstStageMask = dst_stage_mask,
            .dstAccessMask = dst_access_mask,
            .oldLayout = old_layout,
            .newLayout = new_layout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image,
            .subresourceRange = subresource_range
        };

        const VkDependencyInfo dependency_info{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .dependencyFlags = 0,
            .memoryBarrierCount = 0,
            .pMemoryBarriers = VK_NULL_HANDLE,
            .bufferMemoryBarrierCount = 0,
            .pBufferMemoryBarriers = VK_NULL_HANDLE,
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &image_memory_barrier
        };

        vkCmdPipelineBarrier2(command_buffer, &dependency_info);
    }

    // void image_layout_transition(VkCommandBuffer                command_buffer,
    //                              VkImage                        image,
    //                              VkImageLayout                  old_layout,
    //                              VkImageLayout                  new_layout,
    //                              VkImageSubresourceRange const& subresource_range)
    // {
    //     VkImageMemoryBarrier2 image_memory_barrier{
    //         .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
    //         .pNext = nullptr,
    //         .srcStageMask = get_pipeline_stage_flags(old_layout),
    //         .srcAccessMask = get_access_flags(old_layout),
    //         .dstStageMask = get_pipeline_stage_flags(new_layout),
    //         .dstAccessMask = get_access_flags(new_layout),
    //         .oldLayout = old_layout,
    //         .newLayout = new_layout,
    //         .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    //         .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    //         .image = image,
    //         .subresourceRange = subresource_range
    //     };
    //
    //     const VkDependencyInfo dependency_info{
    //         .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
    //         .dependencyFlags = 0,
    //         .memoryBarrierCount = 0,
    //         .pMemoryBarriers = VK_NULL_HANDLE,
    //         .bufferMemoryBarrierCount = 0,
    //         .pBufferMemoryBarriers = VK_NULL_HANDLE,
    //         .imageMemoryBarrierCount = 1,
    //         .pImageMemoryBarriers = &image_memory_barrier
    //     };
    //
    //     vkCmdPipelineBarrier2(command_buffer, &dependency_info);
    // }

    // VkPipelineStageFlags2 get_pipeline_stage_flags(VkImageLayout layout)
    // {
    //     switch (layout)
    //     {
    //         case VK_IMAGE_LAYOUT_UNDEFINED:
    //             return VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
    //         case VK_IMAGE_LAYOUT_PREINITIALIZED:
    //             return VK_PIPELINE_STAGE_2_HOST_BIT;
    //         case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
    //         case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
    //             return VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    //         case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
    //             return VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    //         case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
    //             return VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
    //         case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
    //             return VK_PIPELINE_STAGE_2_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
    //         case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
    //             return VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    //         case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
    //             return VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
    //         case VK_IMAGE_LAYOUT_GENERAL:
    //             assert(false && "Don't know how to get a meaningful VkPipelineStageFlags for VK_IMAGE_LAYOUT_GENERAL! Don't use it!");
    //             return 0;
    //         default:
    //             assert(false);
    //             return 0;
    //     }
    // }
    //
    // VkAccessFlags2 get_access_flags(VkImageLayout layout)
    // {
    //     switch (layout)
    //     {
    //         case VK_IMAGE_LAYOUT_UNDEFINED:
    //         case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
    //             return 0;
    //         case VK_IMAGE_LAYOUT_PREINITIALIZED:
    //             return VK_ACCESS_2_HOST_WRITE_BIT;
    //         case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
    //             return VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    //         case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
    //             return VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    //         case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
    //             return VK_ACCESS_2_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR;
    //         case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
    //             return VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT;
    //         case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
    //             return VK_ACCESS_2_TRANSFER_READ_BIT;
    //         case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
    //             return VK_ACCESS_2_TRANSFER_WRITE_BIT;
    //         case VK_IMAGE_LAYOUT_GENERAL:
    //             assert(false && "Don't know how to get a meaningful VkAccessFlags for VK_IMAGE_LAYOUT_GENERAL! Don't use it!");
    //             return 0;
    //         default:
    //             assert(false);
    //             return 0;
    //     }
    // }
} // namespace pvp
