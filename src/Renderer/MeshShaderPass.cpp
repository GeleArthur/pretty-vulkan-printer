#include "MeshShaderPass.h"

#include <VulkanExternalFunctions.h>
#include <GraphicsPipeline/PipelineLayoutBuilder.h>
#include <tracy/Tracy.hpp>

#include "RenderInfoBuilder.h"
#include "Swapchain.h"
#include "DescriptorSets/DescriptorLayoutBuilder.h"
#include "Scene/PVPScene.h"

#include <Image/ImageBuilder.h>
#include <Image/TransitionLayout.h>

struct DrawCommandIndirect
{
    uint32_t group_count_x;
    uint32_t group_count_y;
    uint32_t group_count_z;
    uint32_t mesh_let_offset;
};

pvp::MeshShaderPass::MeshShaderPass(const Context& context, const PvpScene& scene)
    : m_context(context)
    , m_scene(scene)
{
    create_images();
    build_draw_calls();
    build_pipelines();

    VkQueryPoolCreateInfo pool{
        .sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queryType = VK_QUERY_TYPE_MESH_PRIMITIVES_GENERATED_EXT,
        .queryCount = 1,
        .pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_TASK_SHADER_INVOCATIONS_BIT_EXT | VK_QUERY_PIPELINE_STATISTIC_MESH_SHADER_INVOCATIONS_BIT_EXT
    };
    vkCreateQueryPool(context.device->get_device(), &pool, nullptr, &m_query_pool);
    m_destructor_queue.add_to_queue([&] { vkDestroyQueryPool(m_context.device->get_device(), m_query_pool, nullptr); });
}

void pvp::MeshShaderPass::draw(const FrameContext& cmd, uint32_t swapchain_image_index)
{
    ZoneScoped;

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

    vkCmdResetQueryPool(cmd.command_buffer, m_query_pool, 0, 1);

    vkCmdBeginQuery(cmd.command_buffer, m_query_pool, 0, 0);

    RenderInfoBuilderOut render_color_info;
    RenderInfoBuilder{}
        .add_color(m_context.swapchain->get_views()[swapchain_image_index], VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE)
        .set_depth(m_depth_image.get_view(cmd), VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE)
        .set_size(m_context.swapchain->get_swapchain_extent())
        .build(render_color_info);

    vkCmdBeginRendering(cmd.command_buffer, &render_color_info.rendering_info);

    if (!m_use_indirect)
    {
        vkCmdBindPipeline(cmd.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
        vkCmdBindDescriptorSets(cmd.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout, 0, 1, m_scene.get_scene_descriptor().get_descriptor_set(cmd), 0, nullptr);

        for (const Model& model : m_scene.get_models())
        {
            ZoneScopedN("Draw");
            vkCmdPushConstants(cmd.command_buffer, m_pipeline_layout, VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_TASK_BIT_EXT, 0, sizeof(MaterialTransform), &model.material);
            vkCmdBindDescriptorSets(cmd.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout, 1, 1, model.meshlet_descriptor_set.get_descriptor_set(cmd), 0, nullptr);
            uint32_t thread_group_count_x = model.meshlet_count / 32 + 1;
            VulkanInstanceExtensions::vkCmdDrawMeshTasksEXT(cmd.command_buffer, thread_group_count_x, 1, 1);
            // VulkanInstanceExtensions::vkCmdDrawMeshTasksIndirectEXT()
            // vkCmdDrawMeshTasksIndirectEXT()
            // VkDrawMeshTasksIndirectCommandEXT{};
        }
    }
    else // Use indirect
    {
        vkCmdBindPipeline(cmd.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_indirect);
        vkCmdBindDescriptorSets(cmd.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout_indirect, 0, 1, m_scene.get_scene_descriptor().get_descriptor_set(cmd), 0, nullptr);
        vkCmdBindDescriptorSets(cmd.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout_indirect, 1, 1, m_indirect_descriptor.get_descriptor_set(cmd), 0, nullptr);

        VulkanInstanceExtensions::vkCmdDrawMeshTasksIndirectEXT(cmd.command_buffer, m_gpu_indirect_draw_calls.get_buffer(), 0, m_scene.get_models().size(), sizeof(DrawCommandIndirect));
    }

    vkCmdEndRendering(cmd.command_buffer);
    vkCmdEndQuery(cmd.command_buffer, m_query_pool, 0);
}

// TODO: replace
std::array<uint64_t, 2> pvp::MeshShaderPass::get_invocations_count() const
{
    std::array<uint64_t, 2> data{};
    vkGetQueryPoolResults(m_context.device->get_device(), m_query_pool, 0, 1, sizeof(uint64_t) * data.size(), data.data(), sizeof(uint64_t), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
    return data;
}

void pvp::MeshShaderPass::build_pipelines()
{
    ZoneScoped;
    PipelineLayoutBuilder()
        .add_descriptor_layout(m_context.descriptor_creator->get_layout().from_tag(DiscriptorTag::scene_globals).get())
        .add_descriptor_layout(m_context.descriptor_creator->get_layout().from_tag(DiscriptorTag::meshlets).get())
        .add_push_constant_range(VkPushConstantRange{ VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_TASK_BIT_EXT, 0, sizeof(MaterialTransform) + sizeof(uint32_t) })
        .build(m_context.device->get_device(), m_pipeline_layout);
    m_destructor_queue.add_to_queue([&] { vkDestroyPipelineLayout(m_context.device->get_device(), m_pipeline_layout, nullptr); });

    GraphicsPipelineBuilder()
        .add_shader("shaders/triangle_simple.task", VK_SHADER_STAGE_TASK_BIT_EXT)
        .add_shader("shaders/triangle_simple.mesh", VK_SHADER_STAGE_MESH_BIT_EXT)
        .add_shader("shaders/triangle_simple.frag", VK_SHADER_STAGE_FRAGMENT_BIT)
        .set_depth_format(m_depth_image.get_format())
        .set_depth_access(VK_TRUE, VK_TRUE)
        .set_color_format(std::array{ m_context.swapchain->get_swapchain_surface_format().format })
        .set_pipeline_layout(m_pipeline_layout)
        .build(*m_context.device, m_pipeline);
    m_destructor_queue.add_to_queue([&] { vkDestroyPipeline(m_context.device->get_device(), m_pipeline, nullptr); });

    PipelineLayoutBuilder()
        .add_descriptor_layout(m_context.descriptor_creator->get_layout().from_tag(DiscriptorTag::scene_globals).get())
        .add_descriptor_layout(m_context.descriptor_creator->get_layout()
                                   .add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT)
                                   .add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT)
                                   .add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT)
                                   .add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT)
                                   .add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT)
                                   .add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT)
                                   .add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT)
                                   .add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT)
                                   .get())
        .build(m_context.device->get_device(), m_pipeline_layout_indirect);
    m_destructor_queue.add_to_queue([&] { vkDestroyPipelineLayout(m_context.device->get_device(), m_pipeline_layout_indirect, nullptr); });

    GraphicsPipelineBuilder()
        .add_shader("shaders/triangle_simple_indirect.task", VK_SHADER_STAGE_TASK_BIT_EXT)
        .add_shader("shaders/triangle_simple_indirect.mesh", VK_SHADER_STAGE_MESH_BIT_EXT)
        .add_shader("shaders/triangle_simple.frag", VK_SHADER_STAGE_FRAGMENT_BIT)
        .set_depth_format(m_depth_image.get_format())
        .set_depth_access(VK_TRUE, VK_TRUE)
        .set_color_format(std::array{ m_context.swapchain->get_swapchain_surface_format().format })
        .set_pipeline_layout(m_pipeline_layout_indirect)
        .build(*m_context.device, m_pipeline_indirect);
    m_destructor_queue.add_to_queue([&] { vkDestroyPipeline(m_context.device->get_device(), m_pipeline_indirect, nullptr); });

    DescriptorSetBuilder{}
        .set_layout(m_context.descriptor_creator->get_layout()
                        .add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT)
                        .add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT)
                        .add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT)
                        .add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT)
                        .add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT)
                        .add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT)
                        .add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT)
                        .add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT)
                        .get())
        .bind_buffer_ssbo(0, m_gpu_indirect_draw_calls)
        .bind_buffer_ssbo(1, m_scene.get_matrix_buffer())
        .bind_buffer_ssbo(2, m_scene.get_meshlets_buffer())
        .bind_buffer_ssbo(3, m_scene.get_all_vertex_buffer())
        .bind_buffer_ssbo(4, m_scene.get_meshlets_vertices_buffer())
        .bind_buffer_ssbo(5, m_scene.get_meshlets_triangles_buffer())
        .bind_buffer_ssbo(6, m_scene.get_meshlets_sphere_bounds_buffer())
        .build(m_context, m_indirect_descriptor);
}

void pvp::MeshShaderPass::create_images()
{
    ImageBuilder()
        .set_format(VK_FORMAT_D32_SFLOAT)
        .set_aspect_flags(VK_IMAGE_ASPECT_DEPTH_BIT)
        .set_memory_usage(VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE)
        .set_screen_size_auto_update(true)
        .set_usage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
        .build(m_context, m_depth_image);
    m_destructor_queue.add_to_queue([&] { m_depth_image.destroy(m_context); });
}
void pvp::MeshShaderPass::build_draw_calls()
{
    const std::vector<Model>& models = m_scene.get_models();
    BufferBuilder{}
        .set_size(sizeof(DrawCommandIndirect) * models.size())
        .set_memory_usage(VMA_MEMORY_USAGE_AUTO)
        .set_usage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT)
        .set_flags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT)
        .build(m_context.allocator->get_allocator(), m_gpu_indirect_draw_calls);
    m_destructor_queue.add_to_queue([&] { m_gpu_indirect_draw_calls.destroy(); });

    DrawCommandIndirect* buffer_array = static_cast<DrawCommandIndirect*>(m_gpu_indirect_draw_calls.get_allocation_info().pMappedData);
    uint32_t             meshlet_offset{};
    for (int i = 0; i < models.size(); ++i)
    {
        uint32_t thread_group_count_x = models[i].meshlet_count / 32 + 1;
        buffer_array[i] = DrawCommandIndirect{ thread_group_count_x, 1, 1, meshlet_offset };
        meshlet_offset += models[i].meshlet_count;
    }
}
