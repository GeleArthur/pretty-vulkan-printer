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
    class ToneMappingPass;

    class MeshShaderPass
    {
    public:
        explicit MeshShaderPass(const Context& contexmt, ToneMappingPass& tone_mapping_pass);
        void   draw(const FrameContext& cmd, uint32_t swapchain_image_index);
        ~MeshShaderPass() = default;
        DISABLE_COPY(MeshShaderPass);
        DISABLE_MOVE(MeshShaderPass);

    private:
        void            build_pipelines();
        const Context&  m_context;

        const ToneMappingPass& m_tone_mapping_pass;
        VkPipelineLayout m_pipeline_layout{};
        VkPipeline       m_pipeline{};

        DestructorQueue m_destructor_queue{};
    };
} // namespace pvp
