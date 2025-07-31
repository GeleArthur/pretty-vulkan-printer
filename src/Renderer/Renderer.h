#pragma once
#include "FrameContext.h"
#include "GBuffer.h"
#include "LightPass.h"
#include "Swapchain.h"

#include <Scene/PVPScene.h>
#include <SyncManager/FrameSyncers.h>
#include <tracy/TracyVulkan.hpp>

namespace pvp
{
    class ToneMappingPass;
    class DepthPrePass;
    class BlitToSwapchain;

    class Renderer
    {
    public:
        explicit Renderer(Context& context, Swapchain& swapchain, PvpScene& scene);

        void prepare_frame();
        void draw();
        void end_frame();

        uint32_t get_current_buffer_index() const
        {
            return m_double_buffer_frame;
        }

    private:
        Context&   m_context;
        Swapchain& m_swapchain;
        PvpScene&  m_scene;

        uint32_t     m_double_buffer_frame{ 0 };
        uint32_t     m_current_swapchain_index{};
        FrameSyncers m_frame_syncers{};

        CommandPool                                    m_cmd_pool_graphics_present{};
        std::array<FrameContext, MAX_FRAMES_IN_FLIGHT> m_frame_contexts;

        DepthPrePass*    m_depth_pre_pass{};
        GBuffer*         m_geometry_draw{};
        LightPass*       m_light_pass{};
        ToneMappingPass* m_tone_mapping_pass{};
        BlitToSwapchain* m_blit_to_swapchain{};

        DestructorQueue m_destructor_queue{};
    };
} // namespace pvp
