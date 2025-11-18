#include "MeshShaderPass.h"

#include <VulkanExternalFunctions.h>
#include <GraphicsPipeline/PipelineLayoutBuilder.h>
#include <tracy/Tracy.hpp>

#include "RenderInfoBuilder.h"
#include "Swapchain.h"
#include "DescriptorSets/DescriptorLayoutBuilder.h"
#include "Scene/PVPScene.h"

#include <Image/TransitionLayout.h>

pvp::MeshShaderPass::MeshShaderPass(const Context& context, const PvpScene& scene)
    : m_context(context)
    , m_scene(scene)
{
    build_pipelines();
}

void pvp::MeshShaderPass::draw(const FrameContext& cmd, uint32_t swapchain_image_index)
{
    // VkImageSubresourceRange range{
    //     .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
    //     .baseMipLevel = 0,
    //     .levelCount = VK_REMAINING_MIP_LEVELS,
    //     .baseArrayLayer = 0,
    //     .layerCount = VK_REMAINING_ARRAY_LAYERS
    // };
    //
    // image_layout_transition(cmd.command_buffer,
    //                         m_context.swapchain->get_images()[swapchain_image_index],
    //                         VK_PIPELINE_STAGE_2_NONE,
    //                         VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
    //                         VK_ACCESS_2_NONE,
    //                         VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
    //                         VK_IMAGE_LAYOUT_UNDEFINED,
    //                         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    //                         range);

    RenderInfoBuilderOut render_color_info;
    RenderInfoBuilder()
        .add_color(m_context.swapchain->get_views()[swapchain_image_index], VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE)
        .set_size(m_context.swapchain->get_swapchain_extent())
        .build(render_color_info);

    vkCmdBeginRendering(cmd.command_buffer, &render_color_info.rendering_info);

    vkCmdBindPipeline(cmd.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
    vkCmdBindDescriptorSets(cmd.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout, 0, 1, m_scene.get_scene_descriptor().get_descriptor_set(cmd), 0, nullptr);

    for (const Model& model : m_scene.get_models())
    {
        ZoneScopedN("Draw");
        vkCmdPushConstants(cmd.command_buffer, m_pipeline_layout, VK_SHADER_STAGE_MESH_BIT_EXT, 0, sizeof(MaterialTransform), &model.material);
        vkCmdBindDescriptorSets(cmd.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout, 1, 1, model.meshlet_descriptor_set.get_descriptor_set(cmd), 0, nullptr);
        VulkanInstanceExtensions::vkCmdDrawMeshTasksEXT(cmd.command_buffer, model.meshlet_count, 1, 1);
    }

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
}

void pvp::MeshShaderPass::build_pipelines()
{
    ZoneScoped;
    // m_context.descriptor_creator->create_layout()
    //     .add_binding(VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
    //     .add_binding(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT)
    //     .add_binding(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT)
    //     .add_binding(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT)
    //     .add_binding(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT)
    //     .build(11);

    // m_destructor_queue.add_to_queue([&] { m_context.descriptor_creator->remove_layout(11); });

    // SamplerBuilder()
    //     .set_filter(VK_FILTER_LINEAR)
    //     .set_address_mode(VK_SAMPLER_ADDRESS_MODE_REPEAT)
    //     .build(m_context, m_sampler);
    // m_destructor_queue.add_to_queue([&] { vkDestroySampler(m_context.device->get_device(), m_sampler.handle, nullptr); });

    // DescriptorSetBuilder()
    //     .bind_sampler(0, m_sampler)
    //     .bind_image(1, m_geometry_pass.get_albedo_image(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    //     .bind_image(2, m_geometry_pass.get_normal_image(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    //     .bind_image(3, m_geometry_pass.get_metal_roughness_image(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    //     .bind_image(4, m_depth_pre_pass.get_depth_image(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    //     .set_layout(m_context.descriptor_creator->get_layout(11))
    //     .build(m_context, m_light_binding);

    PipelineLayoutBuilder()
        .add_descriptor_layout(m_context.descriptor_creator->get_layout(0))
        .add_descriptor_layout(m_context.descriptor_creator->get_layout(17))
        .add_push_constant_range(VkPushConstantRange{ VK_SHADER_STAGE_MESH_BIT_EXT, 0, sizeof(MaterialTransform) })
        .build(m_context.device->get_device(), m_pipeline_layout);
    m_destructor_queue.add_to_queue([&] { vkDestroyPipelineLayout(m_context.device->get_device(), m_pipeline_layout, nullptr); });

    GraphicsPipelineBuilder()
        .add_shader("shaders/triangle_simple.mesh", VK_SHADER_STAGE_MESH_BIT_EXT)
        .add_shader("shaders/triangle_simple.frag", VK_SHADER_STAGE_FRAGMENT_BIT)
        .set_pipeline_layout(m_pipeline_layout)
        .build(*m_context.device, m_pipeline);
    m_destructor_queue.add_to_queue([&] { vkDestroyPipeline(m_context.device->get_device(), m_pipeline, nullptr); });
}
