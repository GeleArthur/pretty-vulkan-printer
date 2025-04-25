#pragma once
#include <Buffer.h>
#include <LoadModel.h>
#include <PVPInstance/PVPInstance.h>
#include <PVPPhysicalDevice/PVPPhysicalDevice.h>
#include <PVPSyncManager/Fence.h>
#include <PVPSyncManager/Semaphore.h>
#include <PVPSyncManager/SyncBuilder.h>
#include <PVPVMAAllocator/VmaAllocator.h>

namespace pvp
{
    class CommandBuffer;

    class Swapchain;
    class App
    {
        public:
        void run();
        void draw_frame();
        void record_commands(VkCommandBuffer graphics_command, uint32_t image_index);

        private:
        Instance*             m_pvp_instance {};
        PhysicalDevice*       m_pvp_physical_device {};
        Swapchain*            m_pvp_swapchain {};
        VkRenderPass          m_pvp_render_pass {};
        VkDescriptorSetLayout m_descriptor_set_layout {};
        VkPipelineLayout      m_pipeline_layout {};
        PvpVmaAllocator*      m_allocator {};
        VkPipeline            m_graphics_pipeline {};
        CommandBuffer*        m_command_buffer {};
        Buffer*               m_vertex_buffer {};
        SyncBuilder*          m_sync_builder {};
        LoadModel             m_model {};
        DestructorQueue       m_destructor_queue;

        Semaphore             m_image_available_semaphore {};
        Semaphore             m_render_finished_semaphore {};
        Fence                 m_in_flight_fence {};
    };

} // namespace pvp
