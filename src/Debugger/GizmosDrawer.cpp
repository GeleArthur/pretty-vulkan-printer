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

#include "Scene/PVPScene.h"

pvp::GizmosDrawer::GizmosDrawer(Context& context, const PvpScene& scene)
    : m_context{ context }
    , m_scene{ scene }
{
    build_buffers();
    build_pipelines();
}

void pvp::GizmosDrawer::draw(const FrameContext& cmd, uint32_t swapchain_image_index)
{
    if (m_sphere_count == 0)
    {
        return;
    }

    RenderInfoBuilderOut render_color_info;
    RenderInfoBuilder{}
        .add_color(m_context.swapchain->get_views()[swapchain_image_index], VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE)
        .set_size(m_context.swapchain->get_swapchain_extent())
        .build(render_color_info);

    vkCmdBeginRendering(cmd.command_buffer, &render_color_info.rendering_info);

    vkCmdBindPipeline(cmd.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
    vkCmdBindDescriptorSets(cmd.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout, 0, 1, m_scene.get_scene_descriptor().get_descriptor_set(cmd), 0, nullptr);

    // VulkanInstanceExtensions::vkCmdDrawMeshTasksEXT(cmd.command_buffer, m_sphere_count, 1, 1);

    for (const Model& model : m_scene.get_models())
    {
        vkCmdBindDescriptorSets(cmd.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout, 1, 1, model.meshlet_descriptor_set.get_descriptor_set(cmd), 0, nullptr);
        vkCmdPushConstants(cmd.command_buffer, m_pipeline_layout, VK_SHADER_STAGE_MESH_BIT_EXT, 0, sizeof(MaterialTransform), &model.material);
        VulkanInstanceExtensions::vkCmdDrawMeshTasksEXT(cmd.command_buffer, model.meshlet_count, 1, 1);
    }

    vkCmdEndRendering(cmd.command_buffer);

    // VkImageSubresourceRange range{
    //     .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
    //     .baseMipLevel = 0,
    //     .levelCount = VK_REMAINING_MIP_LEVELS,
    //     .baseArrayLayer = 0,
    //     .layerCount = VK_REMAINING_ARRAY_LAYERS
    // };

    // image_layout_transition(cmd.command_buffer,
    //                         m_context.swapchain->get_images()[swapchain_image_index],
    //                         VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
    //                         VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
    //                         VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT,
    //                         VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
    //                         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    //                         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    //                         range);

    m_sphere_count = 0;
}

void pvp::GizmosDrawer::draw_sphere(const GizmosSphere& sphere)
{
    static_cast<GizmosSphere*>(m_sphere_buffer.get_allocation_info().pMappedData)[m_sphere_count++] = sphere;
}

void pvp::GizmosDrawer::build_buffers()
{
    BufferBuilder{}
        .set_memory_usage(VMA_MEMORY_USAGE_AUTO)
        .set_size(sizeof(GizmosSphere) * 10)
        .set_flags(VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT)
        .set_usage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
        .build(m_context.allocator->get_allocator(), m_sphere_buffer);
    m_destructor_queue.add_to_queue([&] { m_sphere_buffer.destroy(); });
}

void pvp::GizmosDrawer::build_pipelines()
{
    // VkDescriptorSetLayout layout = m_context.descriptor_creator->get_layout()
    //     .add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_MESH_BIT_EXT).get();
    //
    // DescriptorSetBuilder{}
    //     .set_layout(layout)
    //     .bind_buffer_ssbo(0, m_sphere_buffer)
    //     .build(m_context, m_sphere_descriptor);

    PipelineLayoutBuilder()
        .add_descriptor_layout(m_context.descriptor_creator->get_layout().from_tag(DiscriptorTag::scene_globals).get())
        .add_descriptor_layout(m_context.descriptor_creator->get_layout().from_tag(DiscriptorTag::meshlets).get())
        .add_push_constant_range(VkPushConstantRange{ VK_SHADER_STAGE_MESH_BIT_EXT, 0, sizeof(MaterialTransform) })

        // .add_descriptor_layout(layout)
        .build(m_context.device->get_device(), m_pipeline_layout);
    m_destructor_queue.add_to_queue([&] {
        vkDestroyPipelineLayout(m_context.device->get_device(), m_pipeline_layout, nullptr);
    });

    GraphicsPipelineBuilder()
        .add_shader("shaders/gizmos.mesh", VK_SHADER_STAGE_MESH_BIT_EXT)
        .add_shader("shaders/gizmos.frag", VK_SHADER_STAGE_FRAGMENT_BIT)
        .set_color_format(std::array{ m_context.swapchain->get_swapchain_surface_format().format })
        .set_pipeline_layout(m_pipeline_layout)
        .build(*m_context.device, m_pipeline);
    m_destructor_queue.add_to_queue([&] { vkDestroyPipeline(m_context.device->get_device(), m_pipeline, nullptr); });
}
