#pragma once
#include <DestructorQueue.h>
#include <globalconst.h>
#include <variant>
#include <Buffer/Buffer.h>
#include <Context/Context.h>
#include <glm/glm.hpp>

struct FrameContext;
namespace pvp
{
    struct GizmosSphere
    {
        glm::vec3 position;
        float     size;
    };

    class GizmosDrawer final
    {
    public:
        explicit GizmosDrawer(Context& context);
        ~GizmosDrawer() = default;
        DISABLE_COPY(GizmosDrawer);
        DISABLE_MOVE(GizmosDrawer);
        void draw(const FrameContext& cmd, uint32_t swapchain_image_index);
        void draw_sphere(const GizmosSphere& sphere);

    private:
        Context& m_context;

        void build_buffers();
        void build_pipelines();

        std::vector<std::variant<GizmosSphere>> m_drawables;

        VkPipelineLayout m_pipeline_layout{};
        VkPipeline       m_pipeline{};
        Buffer           m_sphere_buffer;
        Buffer           m_sphere_staging_buffer;

        DestructorQueue m_destructor_queue;
    };
} // namespace pvp