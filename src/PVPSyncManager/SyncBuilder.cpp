#include "SyncBuilder.h"

#include "Fence.h"
#include "Semaphore.h"

namespace pvp
{
    SyncBuilder::SyncBuilder(const VkDevice device)
        : m_device(device)
    {
    }
    Semaphore SyncBuilder::create_semaphore() const
    {
        VkSemaphoreCreateInfo semaphore_info {};
        semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        Semaphore result;
        result.device = m_device;
        vkCreateSemaphore(m_device, &semaphore_info, nullptr, &result.handle);
        return result;
    }
    Fence SyncBuilder::create_fence(const bool signaled) const
    {
        VkFenceCreateInfo fence_info {};
        fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_info.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

        Fence result;
        result.device = m_device;
        vkCreateFence(m_device, &fence_info, nullptr, &result.handle);
        return result;
    }
} // namespace pvp