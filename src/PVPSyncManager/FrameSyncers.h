#pragma once
#include "Fence.h"
#include "Semaphore.h"
#include <PVPPhysicalDevice/Context.h>

#include <array>
#include <globalconst.h>

struct FrameSyncers
{
    explicit FrameSyncers(const pvp::Context& context);
    void destroy(VkDevice device) const;

    std::array<Semaphore, MAX_FRAMES_IN_FLIGHT> image_available_semaphores;
    std::array<Semaphore, MAX_FRAMES_IN_FLIGHT> render_finished_semaphores;
    std::array<Fence, MAX_FRAMES_IN_FLIGHT>     in_flight_fences;
};
