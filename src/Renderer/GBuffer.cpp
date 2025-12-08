#include "GBuffer.h"

#include "DepthPrePass.h"
#include "FrameContext.h"
#include "RenderInfoBuilder.h"
#include "Swapchain.h"

#include <UniformBufferStruct.h>
#include <array>
#include <Debugger/debugger.h>
#include <DescriptorSets/DescriptorLayoutBuilder.h>
#include <DescriptorSets/DescriptorSetBuilder.h>
#include <GraphicsPipeline/GraphicsPipelineBuilder.h>
#include <GraphicsPipeline/PipelineLayoutBuilder.h>
#include <GraphicsPipeline/Vertex.h>
#include <Image/ImageBuilder.h>
#include <Scene/PVPScene.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <tracy/TracyVulkan.hpp>
#include <tracy/Tracy.hpp>

pvp::GBuffer::GBuffer(const Context& context, const PvpScene& scene, DepthPrePass& pass)
    : m_context(context)
    , m_scene(scene)
    , m_depth_pre_pass{ pass }
{
    ZoneScoped;
    create_images();
    build_pipelines();
}

void pvp::GBuffer::build_pipelines()
{
    ZoneScoped;
    PipelineLayoutBuilder()
        .add_descriptor_layout(m_context.descriptor_creator->get_layout().from_tag(DiscriptorTag::scene_globals).get())
        .add_descriptor_layout(m_context.descriptor_creator->get_layout().from_tag(DiscriptorTag::bindless_textures).get())
        .add_push_constant_range(VkPushConstantRange{ VK_SHADER_STAGE_ALL, 0, sizeof(MaterialTransform) })
        .build(m_context.device->get_device(), m_pipeline_layout);
    m_destructor_queue.add_to_queue([&] { vkDestroyPipelineLayout(m_context.device->get_device(), m_pipeline_layout, nullptr); });

    GraphicsPipelineBuilder()
        .add_shader("shaders/gpass.vert", VK_SHADER_STAGE_VERTEX_BIT)
        .add_shader("shaders/gpass.frag", VK_SHADER_STAGE_FRAGMENT_BIT)
        .set_color_format(std::array{ m_albedo_image.get_format(), m_normal_image.get_format(), m_metal_roughness_image.get_format() })
        .set_depth_format(m_depth_pre_pass.get_depth_image().get_format())
        .set_pipeline_layout(m_pipeline_layout)
        .set_input_attribute_description(Vertex::get_attribute_descriptions())
        .set_input_binding_description(Vertex::get_binding_description())
        .set_depth_access(VK_TRUE, VK_FALSE)
        .build(*m_context.device, m_albedo_pipeline);
    m_destructor_queue.add_to_queue([&] { vkDestroyPipeline(m_context.device->get_device(), m_albedo_pipeline, nullptr); });
}

void pvp::GBuffer::create_images()
{
    ZoneScoped;
    ImageBuilder()
        .set_format(m_context.swapchain->get_swapchain_surface_format().format)
        .set_aspect_flags(VK_IMAGE_ASPECT_COLOR_BIT)
        .set_memory_usage(VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE)
        .set_screen_size_auto_update(true)
        .set_usage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
        .build(m_context, m_albedo_image);
    m_destructor_queue.add_to_queue([&] { m_albedo_image.destroy(m_context); });

    ImageBuilder()
        .set_format(VK_FORMAT_R16G16_UNORM)
        .set_aspect_flags(VK_IMAGE_ASPECT_COLOR_BIT)
        .set_memory_usage(VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE)
        .set_screen_size_auto_update(true)
        .set_usage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
        .build(m_context, m_normal_image);
    m_destructor_queue.add_to_queue([&] { m_normal_image.destroy(m_context); });

    ImageBuilder()
        .set_format(VK_FORMAT_R8G8B8A8_UNORM)
        .set_aspect_flags(VK_IMAGE_ASPECT_COLOR_BIT)
        .set_memory_usage(VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE)
        .set_screen_size_auto_update(true)
        .set_usage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
        .build(m_context, m_metal_roughness_image);
    m_destructor_queue.add_to_queue([&] { m_metal_roughness_image.destroy(m_context); });
}

void pvp::GBuffer::draw(const FrameContext& cmd)
{
    ZoneScoped;
    TracyVkZone(m_context.tracy_ctx[cmd.buffer_index], cmd.command_buffer, "GBuffer");
    debugger::start_debug_label(cmd.command_buffer, "G buffer", { 0, 1, 0 });

    ZoneNamedN(transition, "TransitionLayout", true);
    m_albedo_image.transition_layout(cmd,
                                     VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                     VK_PIPELINE_STAGE_2_NONE,
                                     VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                     VK_ACCESS_2_NONE,
                                     VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);
    m_normal_image.transition_layout(cmd,
                                     VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                     VK_PIPELINE_STAGE_2_NONE,
                                     VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                     VK_ACCESS_2_NONE,
                                     VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);
    m_metal_roughness_image.transition_layout(cmd,
                                              VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                              VK_PIPELINE_STAGE_2_NONE,
                                              VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                              VK_ACCESS_2_NONE,
                                              VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);

    ZoneNamedN(bind_texture, "Bind Scene+Texture", true);
    vkCmdBindDescriptorSets(cmd.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout, 0, 1, m_scene.get_scene_descriptor().get_descriptor_set(cmd), 0, nullptr);
    vkCmdBindDescriptorSets(cmd.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout, 1, 1, m_scene.get_textures_descriptor().get_descriptor_set(cmd), 0, nullptr);

    ZoneNamedN(render_info, "create render info", true);
    RenderInfoBuilderOut color_info;
    RenderInfoBuilder()
        .add_color(m_albedo_image.get_view(cmd), VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE)
        .add_color(m_normal_image.get_view(cmd), VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE)
        .add_color(m_metal_roughness_image.get_view(cmd), VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE)
        .set_depth(m_depth_pre_pass.get_depth_image().get_view(cmd), VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE)
        .set_size(m_albedo_image.get_size())
        .build(color_info);

    ZoneNamedN(begin_rendering, "begin rendering", true);
    vkCmdBeginRendering(cmd.command_buffer, &color_info.rendering_info);
    {
        vkCmdBindPipeline(cmd.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_albedo_pipeline);
        for (const Model& model : m_scene.get_models())
        {
            ZoneScopedN("Draw");
            VkDeviceSize offset{ 0 };
            vkCmdBindVertexBuffers(cmd.command_buffer, 0, 1, &model.vertex_data.get_buffer(), &offset);
            vkCmdBindIndexBuffer(cmd.command_buffer, model.index_data.get_buffer(), 0, VK_INDEX_TYPE_UINT32);
            vkCmdPushConstants(cmd.command_buffer, m_pipeline_layout, VK_SHADER_STAGE_ALL, 0, sizeof(MaterialTransform), &model.material);
            vkCmdDrawIndexed(cmd.command_buffer, model.index_count, 1, 0, 0, 0);
        }
    }
    ZoneNamedN(end_rendering, "end rendering", true);
    vkCmdEndRendering(cmd.command_buffer);

    ZoneNamedN(transition_depth, "transition", true);
    m_albedo_image.transition_layout(cmd,
                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                     VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                     VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                                     VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                                     VK_ACCESS_2_SHADER_READ_BIT);

    m_normal_image.transition_layout(cmd,
                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                     VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                     VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                                     VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                                     VK_ACCESS_2_SHADER_READ_BIT);
    m_metal_roughness_image.transition_layout(cmd,
                                              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                              VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                              VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                                              VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                                              VK_ACCESS_2_SHADER_READ_BIT);

    m_depth_pre_pass.get_depth_image().transition_layout(cmd,
                                                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                         VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                                                         VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                                                         VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                                                         VK_ACCESS_2_SHADER_READ_BIT);
    debugger::end_debug_label(cmd.command_buffer);
}
