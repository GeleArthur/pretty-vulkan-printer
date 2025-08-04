#include "FrameSyncers.h"

#include <globalconst.h>
#include <Context/Device.h>
#include <spdlog/spdlog.h>
#include <tracy/Tracy.hpp>

FrameSyncers::FrameSyncers(const pvp::Context& context)
{
    ZoneScoped;
    for (int i = 0; i < max_frames_in_flight; ++i)
    {
        VkSemaphoreCreateInfo semaphore_info{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

        vkCreateSemaphore(context.device->get_device(), &semaphore_info, nullptr, &image_available_semaphores[i].handle);
        vkCreateSemaphore(context.device->get_device(), &semaphore_info, nullptr, &render_finished_semaphores[i].handle);

        VkFenceCreateInfo fence_info{ .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT };

        vkCreateFence(context.device->get_device(), &fence_info, nullptr, &in_flight_fences[i].handle);
    }
}
void FrameSyncers::destroy(const VkDevice device) const
{
    for (int i = 0; i < max_frames_in_flight; ++i)
    {
        image_available_semaphores[i].destroy(device);
        render_finished_semaphores[i].destroy(device);
        in_flight_fences[i].destroy(device);
    }
}