#pragma once
#include "Swapchain.h"

#include <PVPSyncManager/FrameSyncers.h>

namespace pvp
{
    class Renderer
    {
    public:
        explicit Renderer(const Context& context, Swapchain& swapchain);

        void prepare_frame();
        void end_frame();

    private:
        const Context&               m_context;
        Swapchain&                   m_swapchain;
        uint32_t                     m_double_buffer_frame{ 0 };
        uint32_t                     m_current_swapchain_index{};
        FrameSyncers                 m_frame_syncers{};
        CommandPool                  m_cmd_pool_graphics_present{};
        std::vector<VkCommandBuffer> m_cmds_graphics{};

        DestructorQueue m_destructor_queue{};
    };
} // namespace pvp
