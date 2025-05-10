#pragma once
#include <PVPBuffer/Buffer.h>
#include <PVPImage/Image.h>
#include <PVPWindow/WindowSurface.h>

namespace pvp
{
    class GBuffer final
    {
    public:
        explicit GBuffer(const Context& context)
            : m_context(context)
        {
        }

        void draw(VkCommandBuffer buffer);

    private:
        void           build_pipelines(const RenderingContext& rendering_context);
        void           draw_albedo(const RenderingContext& rendering_context);
        const Context& m_context;
        const Image    m_albedo_image{};

        VkPipelineLayout m_albedo_pipeline_layout{};
        VkPipeline       m_albedo_pipeline{};
    };
} // namespace pvp
