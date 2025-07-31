#pragma once
#include <DestructorQueue.h>
#include <DescriptorSets/DescriptorSets.h>
#include <Image/Image.h>
#include <vulkan/vulkan.h>
namespace pvp
{
    struct Context;
    class LightPass;
    class ToneMappingPass
    {
    public:
        explicit ToneMappingPass(const Context& context, LightPass& light_pass);
        void   draw(const FrameContext& cmd);
        Image& get_tone_mapped_texture()
        {
            return m_tone_texture;
        };

    private:
        void           build_pipelines();
        void           create_images();
        const Context& m_context;
        LightPass&     m_light_pass;
        Image          m_tone_texture{};
        DescriptorSets m_tone_binding{};

        VkPipelineLayout m_tone_pipeline_layout{};
        VkPipeline       m_tone_pipeline{};

        DestructorQueue m_destructor_queue{};
    };
} // namespace pvp
