#include "MeshShaderPass.h"

#include <VulkanExternalFunctions.h>
#include <GraphicsPipeline/PipelineLayoutBuilder.h>
#include <tracy/Tracy.hpp>

#include "RenderInfoBuilder.h"
#include "Swapchain.h"

pvp::MeshShaderPass::MeshShaderPass(const Context& context, ToneMappingPass& tone_mapping_pass) :
    m_context(context),
    m_tone_mapping_pass(tone_mapping_pass)
{
    build_pipelines();
}

void pvp::MeshShaderPass::draw(const FrameContext& cmd, uint32_t swapchain_image_index)
{
    RenderInfoBuilderOut render_color_info;
    RenderInfoBuilder()
        .add_color(m_context.swapchain->get_views()[swapchain_image_index], VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE)
        .set_size(m_context.swapchain->get_swapchain_extent())
        .build(render_color_info);

    vkCmdBeginRendering(cmd.command_buffer, &render_color_info.rendering_info);

    vkCmdBindPipeline(cmd.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
    VulkanInstanceExtensions::vkCmdDrawMeshTasksEXT(cmd.command_buffer, 1, 1, 1);

    vkCmdEndRendering(cmd.command_buffer);
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
        .build(m_context.device->get_device(), m_pipeline_layout);
    m_destructor_queue.add_to_queue([&] { vkDestroyPipelineLayout(m_context.device->get_device(), m_pipeline_layout, nullptr); });

    GraphicsPipelineBuilder()
        .add_shader("shaders/triangle_simple.mesh", VK_SHADER_STAGE_MESH_BIT_EXT)
        .add_shader("shaders/triangle_simple.frag", VK_SHADER_STAGE_FRAGMENT_BIT)
        .set_pipeline_layout(m_pipeline_layout)
        .build(*m_context.device, m_pipeline);
    m_destructor_queue.add_to_queue([&] { vkDestroyPipeline(m_context.device->get_device(), m_pipeline, nullptr); });
}
