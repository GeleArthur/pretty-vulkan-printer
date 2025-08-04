#pragma once
#include "Fence.h"
#include "Semaphore.h"
#include <Context/Context.h>

#include <array>
#include <globalconst.h>

struct FrameSyncers
{
    explicit FrameSyncers() = default;
    explicit FrameSyncers(const pvp::Context& context);
    void destroy(VkDevice device) const;

    std::array<Semaphore, max_frames_in_flight> image_available_semaphores;
    std::array<Semaphore, max_frames_in_flight> render_finished_semaphores;
    std::array<Fence, max_frames_in_flight>     in_flight_fences;
};
