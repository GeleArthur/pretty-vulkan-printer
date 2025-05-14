#include "DepthPrePass.h"

namespace pvp
{
    DepthPrePass::DepthPrePass(const Context& context, const PvpScene& scene, const ImageInfo& image_info)
        : m_context{ context }
        , m_scene{ scene }
        , m_image_info{ image_info }
        , m_camera_uniform{}
    {
    }
    void DepthPrePass::draw(VkCommandBuffer cmd)
    {
    }
    void DepthPrePass::build_pipelines()
    {
    }
    void DepthPrePass::create_images()
    {
    }
} // namespace pvp