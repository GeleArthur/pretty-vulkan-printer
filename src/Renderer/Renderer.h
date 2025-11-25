#pragma once
#include "BlitToSwapchain.h"
#include "DepthPrePass.h"
#include "FrameContext.h"
#include "GBuffer.h"
#include "LightPass.h"
#include "Swapchain.h"

#include <SyncManager/FrameSyncers.h>

#include "MeshShaderPass.h"
#include "ToneMappingPass.h"

namespace pvp
{
    class ImguiRenderer;
    class ToneMappingPass;
    class DepthPrePass;
    class BlitToSwapchain;

    class Renderer
    {
    public:
        explicit Renderer(Context& context, PvpScene& scene, ImguiRenderer& imgui_renderer);

        ~Renderer() = default;
        DISABLE_COPY(Renderer);
        DISABLE_MOVE(Renderer);

        void prepare_frame();
        void draw();
        void end_frame();

        [[nodiscard]] uint32_t get_current_buffer_index() const
        {
            return m_double_buffer_frame;
        }

    private:
        Context&  m_context;
        PvpScene& m_scene;

        uint32_t     m_double_buffer_frame{ 0 };
        uint32_t     m_current_swapchain_index{};
        FrameSyncers m_frame_syncers{};

        CommandPool                                    m_cmd_pool_graphics_present{};
        std::array<FrameContext, max_frames_in_flight> m_frame_contexts{};

        DepthPrePass    m_depth_pre_pass;
        GBuffer         m_geometry_draw;
        LightPass       m_light_pass;
        ToneMappingPass m_tone_mapping_pass;
        ImguiRenderer&  m_imgui_renderer;
        BlitToSwapchain m_blit_to_swapchain;
        MeshShaderPass  m_mesh_shader_pass;

        DestructorQueue m_destructor_queue{};
    };
} // namespace pvp
