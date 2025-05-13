#pragma once
#include "GBuffer.h"

#include <DestructorQueue.h>
#include <DescriptorSets/DescriptorSets.h>
#include <GraphicsPipeline/GraphicsPipelineBuilder.h>
#include <Image/Image.h>
#include <Context/Context.h>
#include <UniformBuffers/UniformBuffer.h>
namespace pvp
{
    class LightPass
    {
    public:
        explicit LightPass(const Context& context, const ImageInfo& image_info, GBuffer& gbuffer);
        void   draw(VkCommandBuffer command_buffer);
        Image& get_light_image()
        {
            return m_light_image;
        };

    private:
        void            build_pipelines();
        void            create_images();
        const Context&  m_context;
        GBuffer&        m_gemotry_pass;
        Image           m_light_image{};
        Sampler         m_sampler{};
        const ImageInfo m_image_info;

        DescriptorSets          m_descriptor_binding;
        vk::DescriptorSetLayout m_desciptor_layout;

        UniformBuffer<glm::vec2>* m_screensize_buffer;

        VkPipelineLayout m_light_pipeline_layout{};
        VkPipeline       m_light_pipeline{};

        DestructorQueue m_destructor_queue{};
    };
} // namespace pvp
