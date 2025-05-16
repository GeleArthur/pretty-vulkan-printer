#pragma once
#include "GBuffer.h"
#include "LightPass.h"
#include "Swapchain.h"

#include <SyncManager/FrameSyncers.h>

namespace pvp
{
    class DepthPrePass;
}
namespace pvp
{
    class BlitToSwapchain;
}

namespace pvp
{
    class Renderer
    {
    public:
        explicit Renderer(const Context& context, Swapchain& swapchain, const PvpScene& scene);

        void prepare_frame();
        void draw();
        void end_frame();

        uint32_t get_current_buffer_index() const
        {
            return m_double_buffer_frame;
        }

    private:
        const Context&  m_context;
        Swapchain&      m_swapchain;
        const PvpScene& m_scene;

        uint32_t     m_double_buffer_frame{ 0 };
        uint32_t     m_current_swapchain_index{};
        FrameSyncers m_frame_syncers{};

        CommandPool                  m_cmd_pool_graphics_present{};
        std::vector<VkCommandBuffer> m_cmds_graphics{};

        DepthPrePass*    m_depth_pre_pass{};
        GBuffer*         m_geometry_draw{};
        LightPass*       m_light_pass{};
        BlitToSwapchain* m_blit_to_swapchain{};
        DescriptorSets   m_scene_binding;
        DescriptorSets   m_all_textures;

        DestructorQueue m_destructor_queue{};
    };
} // namespace pvp
