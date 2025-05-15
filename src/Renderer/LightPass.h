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
        explicit LightPass(const Context& context, GBuffer& gbuffer);
        void   draw(VkCommandBuffer command_buffer);
        Image& get_light_image()
        {
            return m_light_image;
        };

    private:
        void           build_pipelines();
        void           create_images();
        const Context& m_context;
        GBuffer&       m_gemotry_pass;
        Image          m_light_image{};
        Sampler        m_sampler{};

        DescriptorSets m_descriptor_binding;

        // UniformBuffer<glm::vec2>* m_screensize_buffer;

        VkPipelineLayout m_light_pipeline_layout{};
        VkPipeline       m_light_pipeline{};

        DestructorQueue m_destructor_queue{};
    };
} // namespace pvp
