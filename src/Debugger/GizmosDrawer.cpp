#include "GizmosDrawer.h"

#include <VulkanExternalFunctions.h>
#include <iomanip>
#include <Context/Device.h>
#include <DescriptorSets/CommonDescriptorLayouts.h>
#include <DescriptorSets/DescriptorLayoutBuilder.h>
#include <DescriptorSets/DescriptorSetBuilder.h>
#include <GraphicsPipeline/GraphicsPipelineBuilder.h>
#include <GraphicsPipeline/PipelineLayoutBuilder.h>
#include <Image/SamplerBuilder.h>
#include <Image/TransitionLayout.h>
#include <Renderer/RenderInfoBuilder.h>
#include <Renderer/Swapchain.h>
pvp::GizmosDrawer::GizmosDrawer(Context& context)
    : m_context{ context }
{
    build_pipelines();
    build_buffers();
}

void pvp::GizmosDrawer::draw(const FrameContext& cmd, uint32_t swapchain_image_index)
{
    if (m_drawables.empty())
        return;

    RenderInfoBuilderOut render_color_info;
    RenderInfoBuilder{}
        .add_color(m_context.swapchain->get_views()[swapchain_image_index], VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE)
        .set_size(m_context.swapchain->get_swapchain_extent())
        .build(render_color_info);

    vkCmdBeginRendering(cmd.command_buffer, &render_color_info.rendering_info);

    vkCmdBindPipeline(cmd.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
    vkCmdBindDescriptorSets(cmd.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout, 0, 1, m_scene.get_scene_descriptor().get_descriptor_set(cmd), 0, nullptr);

    {
        VulkanInstanceExtensions::vkCmdDrawMeshTasksEXT(cmd.command_buffer, thread_group_count_x, 1, 1);
    }

    // for (const Model& model : m_scene.get_models())
    // {
    //     ZoneScopedN("Draw");
    //     vkCmdPushConstants(cmd.command_buffer, m_pipeline_layout, VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_TASK_BIT_EXT, 0, sizeof(MaterialTransform), &model.material);
    //     vkCmdPushConstants(cmd.command_buffer, m_pipeline_layout, VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_TASK_BIT_EXT, sizeof(MaterialTransform), sizeof(uint32_t), &model.meshlet_count);
    //     vkCmdBindDescriptorSets(cmd.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout, 1, 1, model.meshlet_descriptor_set.get_descriptor_set(cmd), 0, nullptr);
    //     uint32_t thread_group_count_x = model.meshlet_count / 32 + 1;
    //     VulkanInstanceExtensions::vkCmdDrawMeshTasksEXT(cmd.command_buffer, thread_group_count_x, 1, 1);
    // }

    vkCmdEndRendering(cmd.command_buffer);

    VkImageSubresourceRange range{
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = VK_REMAINING_MIP_LEVELS,
        .baseArrayLayer = 0,
        .layerCount = VK_REMAINING_ARRAY_LAYERS
    };

    image_layout_transition(cmd.command_buffer,
                            m_context.swapchain->get_images()[swapchain_image_index],
                            VK_PIPELINE_STAGE_2_NONE,
                            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                            VK_ACCESS_2_NONE,
                            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                            VK_IMAGE_LAYOUT_UNDEFINED,
                            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                            range);

    vkCmdEndQuery(cmd.command_buffer, m_query_pool, 0);
}

void pvp::GizmosDrawer::draw_sphere(const GizmosSphere& sphere)
{
    m_drawables.push_back(std::variant<GizmosSphere>(sphere));
}
void pvp::GizmosDrawer::build_buffers()
{
    BufferBuilder{}
        .set_memory_usage(VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE)
        .set_size(sizeof(uint32_t) + sizeof(GizmosSphere) * 10)
        .build(m_context.allocator->get_allocator(), m_sphere_buffer);

    BufferBuilder{}
        .set_memory_usage(VMA_MEMORY_USAGE_AUTO_PREFER_HOST)
        .set_size(sizeof(uint32_t) + sizeof(GizmosSphere) * 10)
        .build(m_context.allocator->get_allocator(), m_sphere_staging_buffer);
}
void pvp::GizmosDrawer::build_pipelines()
{
    PipelineLayoutBuilder()
        .add_descriptor_layout(m_context.descriptor_creator->get_layout().from_tag(DiscriptorTag::scene_globals).get())
        .build(m_context.device->get_device(), m_pipeline_layout);
    m_destructor_queue.add_to_queue([&] { vkDestroyPipelineLayout(m_context.device->get_device(), m_pipeline_layout, nullptr); });

    GraphicsPipelineBuilder()
        .add_shader("shaders/gizmos.vert", VK_SHADER_STAGE_VERTEX_BIT)
        .add_shader("shaders/gizmos.frag", VK_SHADER_STAGE_FRAGMENT_BIT)
        .set_color_format(std::array{ m_context.swapchain->get_swapchain_surface_format().format })
        .set_pipeline_layout(m_pipeline_layout)
        .build(*m_context.device, m_pipeline);
    m_destructor_queue.add_to_queue([&] { vkDestroyPipeline(m_context.device->get_device(), m_pipeline, nullptr); });
}