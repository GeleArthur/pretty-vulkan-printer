#pragma once

#include <vulkan/vulkan.hpp>

namespace pvp
{
    void image_layout_transition(VkCommandBuffer                command_buffer,
                                 VkImage                        image,
                                 VkPipelineStageFlags2          src_stage_mask,
                                 VkPipelineStageFlags2          dst_stage_mask,
                                 VkAccessFlags2                 src_access_mask,
                                 VkAccessFlags2                 dst_access_mask,
                                 VkImageLayout                  old_layout,
                                 VkImageLayout                  new_layout,
                                 VkImageSubresourceRange const& subresource_range);

    // void image_layout_transition(VkCommandBuffer                command_buffer,
    //                              VkImage                        image,
    //                              VkImageLayout                  old_layout,
    //                              VkImageLayout                  new_layout,
    //                              VkImageSubresourceRange const& subresource_range);

    // VkPipelineStageFlags2 get_pipeline_stage_flags(VkImageLayout layout);
    // VkAccessFlags2        get_access_flags(VkImageLayout layout);
} // namespace pvp
