#include "FrameSyncers.h"

#include "Renderer/Swapchain.h"

#include <globalconst.h>
#include <Context/Device.h>
#include <spdlog/spdlog.h>
#include <tracy/Tracy.hpp>

FrameSyncers::FrameSyncers(const pvp::Context& context)
{
    ZoneScoped;
    VkSemaphoreCreateInfo semaphore_info{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    VkFenceCreateInfo fence_info{ .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT };

    for (int i = 0; i < max_frames_in_flight; ++i)
    {
        vkCreateSemaphore(context.device->get_device(), &semaphore_info, nullptr, &acquire_semaphores[i].handle);

        vkCreateFence(context.device->get_device(), &fence_info, nullptr, &in_flight_fences[i].handle);
    }

    for (int i = 0; i < context.swapchain->get_min_image_count(); ++i)
    {
        submit_semaphores.push_back(Semaphore{});
        vkCreateSemaphore(context.device->get_device(), &semaphore_info, nullptr, &submit_semaphores[i].handle);
    }
}

void FrameSyncers::destroy(const VkDevice device) const
{
    for (int i = 0; i < max_frames_in_flight; ++i)
    {
        acquire_semaphores[i].destroy(device);
        in_flight_fences[i].destroy(device);
    }
    for (const Semaphore& render_finished_semaphore : submit_semaphores)
    {
        render_finished_semaphore.destroy(device);
    }
}
