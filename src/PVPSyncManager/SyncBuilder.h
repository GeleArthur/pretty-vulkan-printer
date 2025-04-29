#pragma once

#include <vulkan/vulkan.h>

struct Semaphore;
struct Fence;

namespace pvp
{
    class SyncBuilder
    {
    public:
        explicit SyncBuilder(VkDevice device);
        Semaphore create_semaphore() const;
        Fence     create_fence(bool signaled = false) const;

    private:
        VkDevice m_device;
    };
} // namespace pvp
