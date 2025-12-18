#include "GizmosDrawer.h"

#include "DebugVertex.h"
#include "Gizmos.h"

#include <VulkanExternalFunctions.h>
#include <Context/Device.h>
#include <DescriptorSets/CommonDescriptorLayouts.h>
#include <DescriptorSets/DescriptorLayoutBuilder.h>
#include <DescriptorSets/DescriptorSetBuilder.h>
#include <GraphicsPipeline/GraphicsPipelineBuilder.h>
#include <GraphicsPipeline/PipelineLayoutBuilder.h>
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
    const std::vector<DebugVertex>& lines = gizmos::get_lines();
    std::ranges::copy(lines, static_cast<DebugVertex*>(m_debug_lines_buffer.get_allocation_info().pMappedData));

    RenderInfoBuilderOut render_color_info;
    RenderInfoBuilder{}
        .add_color(m_context.swapchain->get_views()[swapchain_image_index], VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE)
        .set_size(m_context.swapchain->get_swapchain_extent())
        .build(render_color_info);

    vkCmdBeginRendering(cmd.command_buffer, &render_color_info.rendering_info);

    if (m_scene.get_sphere_enabled())
    {
        vkCmdBindPipeline(cmd.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_spheres);
        vkCmdBindDescriptorSets(cmd.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout_spheres, 0, 1, m_scene.get_scene_descriptor().get_descriptor_set(cmd), 0, nullptr);

        for (const Model& model : m_scene.get_models())
        {
            vkCmdBindDescriptorSets(cmd.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout_spheres, 1, 1, model.meshlet_descriptor_set.get_descriptor_set(cmd), 0, nullptr);
            vkCmdPushConstants(cmd.command_buffer, m_pipeline_layout_spheres, VK_SHADER_STAGE_MESH_BIT_EXT, 0, sizeof(MaterialTransform), &model.material);
            VulkanInstanceExtensions::vkCmdDrawMeshTasksEXT(cmd.command_buffer, model.meshlet_count, 1, 1);
        }
    }

    vkCmdBindPipeline(cmd.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_debug_lines);
    vkCmdBindDescriptorSets(cmd.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout_debug_lines, 0, 1, m_scene.get_scene_descriptor().get_descriptor_set(cmd), 0, nullptr);

    VkDeviceSize offset{ 0 };
    vkCmdBindVertexBuffers(cmd.command_buffer, 0, 1, &m_debug_lines_buffer.get_buffer(), &offset);
    vkCmdDraw(cmd.command_buffer, lines.size(), 1, 0, 0);

    vkCmdEndRendering(cmd.command_buffer);
}

void pvp::GizmosDrawer::build_buffers()
{
    BufferBuilder{}
        .set_memory_usage(VMA_MEMORY_USAGE_AUTO)
        .set_size(sizeof(DebugVertex) * 1000)
        .set_flags(VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT)
        .set_usage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
        .build(m_context.allocator->get_allocator(), m_debug_lines_buffer);
    m_destructor_queue.add_to_queue([&] { m_debug_lines_buffer.destroy(); });
}

void pvp::GizmosDrawer::build_pipelines()
{
    PipelineLayoutBuilder()
        .add_descriptor_layout(m_context.descriptor_creator->get_layout().from_tag(DiscriptorTag::scene_globals).get())
        .add_descriptor_layout(m_context.descriptor_creator->get_layout().from_tag(DiscriptorTag::meshlets).get())
        .add_push_constant_range(VkPushConstantRange{ VK_SHADER_STAGE_MESH_BIT_EXT, 0, sizeof(MaterialTransform) })
        .build(m_context.device->get_device(), m_pipeline_layout_spheres);
    m_destructor_queue.add_to_queue([&] {
        vkDestroyPipelineLayout(m_context.device->get_device(), m_pipeline_layout_spheres, nullptr);
    });

    GraphicsPipelineBuilder()
        .add_shader("shaders/gizmos.mesh", VK_SHADER_STAGE_MESH_BIT_EXT)
        .add_shader("shaders/gizmos.frag", VK_SHADER_STAGE_FRAGMENT_BIT)
        .set_color_format(std::array{ m_context.swapchain->get_swapchain_surface_format().format })
        .set_pipeline_layout(m_pipeline_layout_spheres)
        .build(*m_context.device, m_pipeline_spheres);
    m_destructor_queue.add_to_queue([&] { vkDestroyPipeline(m_context.device->get_device(), m_pipeline_spheres, nullptr); });

    PipelineLayoutBuilder()
        .add_descriptor_layout(m_context.descriptor_creator->get_layout().from_tag(DiscriptorTag::scene_globals).get())
        .build(m_context.device->get_device(), m_pipeline_layout_debug_lines);
    m_destructor_queue.add_to_queue([&] { vkDestroyPipelineLayout(m_context.device->get_device(), m_pipeline_layout_debug_lines, nullptr); });

    GraphicsPipelineBuilder()
        .add_shader("shaders/debugline.vert", VK_SHADER_STAGE_VERTEX_BIT)
        .add_shader("shaders/debugline.frag", VK_SHADER_STAGE_FRAGMENT_BIT)
        .set_input_attribute_description(DebugVertex::get_attribute_descriptions())
        .set_input_binding_description(DebugVertex::get_binding_description())
        .set_topology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST)
        .set_cull_mode(VK_CULL_MODE_NONE)
        .set_depth_access(VK_FALSE, VK_FALSE)
        .set_color_format(std::array{ m_context.swapchain->get_swapchain_surface_format().format })
        .set_pipeline_layout(m_pipeline_layout_debug_lines)
        .build(*m_context.device, m_pipeline_debug_lines);
    m_destructor_queue.add_to_queue([&] { vkDestroyPipeline(m_context.device->get_device(), m_pipeline_debug_lines, nullptr); });
}
