#pragma once
#include <PVPInstance/PVPInstance.h>
#include <PVPPhysicalDevice/PVPPhysicalDevice.h>
#include <PVPVMAAllocator/VmaAllocator.h>

struct PVPContext
{
    GLFWwindow*  window { nullptr };
    VkInstance   instance { nullptr };
    VkSurfaceKHR surface { nullptr };
};

namespace pvp
{

    class Swapchain;
    class App
    {
        public:
        void run();

        private:
        Instance*             m_pvp_instance {};
        PhysicalDevice*       m_pvp_physical_device {};
        Swapchain*            m_pvp_swapchain {};
        VkRenderPass          m_pvp_render_pass {};
        VkDescriptorSetLayout m_descriptor_set_layout {};
        VkPipelineLayout      m_pipeline_layout {};
        PvpVmaAllocator*      m_allocator {};
        VkPipeline            m_graphics_pipeline {};
        DestructorQueue       m_destructor_queue;
    };

} // namespace pvp
