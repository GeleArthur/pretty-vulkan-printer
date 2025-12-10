#include "FrameSyncers.h"

#include "Renderer/Swapchain.h"

#include <VulkanExternalFunctions.h>
#include <globalconst.h>
#include <Context/Device.h>
#include <Debugger/debugger.h>
#include <spdlog/spdlog.h>
#include <tracy/Tracy.hpp>

FrameSyncers::FrameSyncers(const pvp::Context& context)
{
    ZoneScoped;
    VkSemaphoreCreateInfo semaphore_info{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    VkFenceCreateInfo     fence_info{ .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT };

    for (uint32_t i = 0; i < max_frames_in_flight; ++i)
    {
        vkCreateFence(context.device->get_device(), &fence_info, nullptr, &in_flight_fences[i].handle);

        acquire_semaphores.push_back(Semaphore{});
        vkCreateSemaphore(context.device->get_device(), &semaphore_info, nullptr, &acquire_semaphores[i].handle);
        pvp::debugger::add_object_name(context.device, acquire_semaphores[i].handle, "acquire_semaphores: " + std::to_string(i));
    }

    for (uint32_t i = 0; i < context.swapchain->get_image_count(); ++i)
    {
        submit_semaphores.push_back(Semaphore{});
        vkCreateSemaphore(context.device->get_device(), &semaphore_info, nullptr, &submit_semaphores[i].handle);
        pvp::debugger::add_object_name(context.device, submit_semaphores[i].handle, "submit_semaphores" + std::to_string(i));
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
