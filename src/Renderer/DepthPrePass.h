#pragma once
#include <DestructorQueue.h>
#include <Context/Context.h>
#include <DescriptorSets/DescriptorSets.h>
#include <Image/Image.h>
#include <Scene/PVPScene.h>
namespace pvp
{
    class DepthPrePass
    {
    public:
        explicit DepthPrePass(const Context& context, const PvpScene& scene);
        void draw(VkCommandBuffer cmd);

        Image& get_depth_image()
        {
            return m_depth_image;
        };

    private:
        void            build_pipelines();
        void            create_images();
        const Context&  m_context;
        const PvpScene& m_scene;
        Image           m_depth_image{};

        VkPipelineLayout m_pipeline_layout{};
        VkPipeline       m_pipeline{};

        DestructorQueue m_destructor_queue{};
    };
} // namespace pvp
