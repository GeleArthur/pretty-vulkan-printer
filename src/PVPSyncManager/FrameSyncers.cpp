#include "FrameSyncers.h"

#include <globalconst.h>
FrameSyncers::FrameSyncers(const pvp::SyncBuilder& builder)
{
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        image_available_semaphores[i] = builder.create_semaphore();
        render_finished_semaphores[i] = builder.create_semaphore();
        in_flight_fences[i] = builder.create_fence(true);
    }
}
void FrameSyncers::destroy(const VkDevice device) const
{
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        image_available_semaphores[i].destroy(device);
        render_finished_semaphores[i].destroy(device);
        in_flight_fences[i].destroy(device);
    }
}