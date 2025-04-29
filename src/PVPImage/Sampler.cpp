#include "Sampler.h"

namespace pvp
{
    void Sampler::destroy(VkDevice device)
    {
        vkDestroySampler(device, handle, nullptr);
    }
} // namespace pvp