#pragma once
#include "GBuffer.h"

#include <DestructorQueue.h>
#include <Context/Context.h>
#include <DescriptorSets/DescriptorSets.h>
#include <GraphicsPipeline/GraphicsPipelineBuilder.h>
#include <Image/Image.h>
#include <UniformBuffers/UniformBuffer.h>
#include <glm/vec2.hpp>
namespace pvp
{
    class LightPass
    {
    public:
        explicit LightPass(const Context& context, const PvpScene& scene, GBuffer& gbuffer, DepthPrePass& depth_pre_pass);
        void   draw(const FrameContext& cmd);
        Image& get_light_image()
        {
            return m_light_image;
        };

    private:
        void            build_pipelines();
        void            create_images();
        const Context&  m_context;
        GBuffer&        m_geometry_pass;
        DepthPrePass&   m_depth_pre_pass;
        const PvpScene& m_scene;
        Image           m_light_image{};
        Sampler         m_sampler{};

        DescriptorSets m_light_binding;

        // UniformBuffer<glm::vec2>* m_screensize_buffer;

        VkPipelineLayout m_light_pipeline_layout{};
        VkPipeline       m_light_pipeline{};

        DestructorQueue m_destructor_queue{};
    };
} // namespace pvp
