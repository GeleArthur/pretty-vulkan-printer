#include "LightPass.h"

#include "DepthPrePass.h"
#include "FrameContext.h"
#include "RenderInfoBuilder.h"
#include "Swapchain.h"

#include <Debugger/debugger.h>
#include <DescriptorSets/DescriptorLayoutBuilder.h>
#include <GraphicsPipeline/PipelineLayoutBuilder.h>
#include <Image/ImageBuilder.h>
#include <Image/SamplerBuilder.h>
#include <Scene/PVPScene.h>
#include <tracy/Tracy.hpp>
#include <tracy/TracyVulkan.hpp>

namespace pvp
{
    LightPass::LightPass(const Context& context, const PvpScene& scene, GBuffer& gbuffer, DepthPrePass& depth_pre_pass)
        : m_context{ context }
        , m_geometry_pass{ gbuffer }
        , m_depth_pre_pass{ depth_pre_pass }
        , m_scene{ scene }
    {
        ZoneScoped;
        create_images();
        build_pipelines();
    }

    void LightPass::build_pipelines()
    {
        ZoneScoped;

        SamplerBuilder()
            .set_filter(VK_FILTER_LINEAR)
            .set_address_mode(VK_SAMPLER_ADDRESS_MODE_REPEAT)
            .build(m_context, m_sampler);
        m_destructor_queue.add_to_queue([&] { vkDestroySampler(m_context.device->get_device(), m_sampler.handle, nullptr); });

        DescriptorSetBuilder()
            .bind_sampler(0, m_sampler)
            .bind_image(1, m_geometry_pass.get_albedo_image(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
            .bind_image(2, m_geometry_pass.get_normal_image(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
            .bind_image(3, m_geometry_pass.get_metal_roughness_image(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
            .bind_image(4, m_depth_pre_pass.get_depth_image(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
            .set_layout(m_context.descriptor_creator->get_layout()
                            .add_binding(VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                            .add_binding(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT)
                            .add_binding(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT)
                            .add_binding(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT)
                            .add_binding(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT)
                            .set_tag(DiscriptorTag::gbuffers)
                            .get())
            .build(m_context, m_texture_binding);

        PipelineLayoutBuilder()
            .add_descriptor_layout(m_context.descriptor_creator->get_layout().from_tag(DiscriptorTag::scene_globals).get())
            .add_descriptor_layout(m_context.descriptor_creator->get_layout().from_tag(DiscriptorTag::gbuffers).get())
            .add_descriptor_layout(m_context.descriptor_creator->get_layout().from_tag(DiscriptorTag::lights).get())
            .build(m_context.device->get_device(), m_light_pipeline_layout);
        m_destructor_queue.add_to_queue([&] { vkDestroyPipelineLayout(m_context.device->get_device(), m_light_pipeline_layout, nullptr); });

        GraphicsPipelineBuilder()
            .add_shader("shaders/lightpass.vert", VK_SHADER_STAGE_VERTEX_BIT)
            .add_shader("shaders/lightpass.frag", VK_SHADER_STAGE_FRAGMENT_BIT)
            .set_color_format(std::array{ m_light_image.get_format() })
            .set_pipeline_layout(m_light_pipeline_layout)
            .build(*m_context.device, m_light_pipeline);
        m_destructor_queue.add_to_queue([&] { vkDestroyPipeline(m_context.device->get_device(), m_light_pipeline, nullptr); });
    }

    void LightPass::create_images()
    {
        ZoneScoped;
        ImageBuilder()
            .set_format(VK_FORMAT_R32G32B32A32_SFLOAT)
            .set_aspect_flags(VK_IMAGE_ASPECT_COLOR_BIT)
            .set_memory_usage(VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE)
            .set_screen_size_auto_update(true)
            .set_usage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
            .build(m_context, m_light_image);
        m_destructor_queue.add_to_queue([&] { m_light_image.destroy(m_context); });
    }

    void LightPass::draw(const FrameContext& cmd)
    {
        ZoneScoped;
        TracyVkZone(m_context.tracy_ctx[cmd.buffer_index], cmd.command_buffer, "LightPass");
        debugger::start_debug_label(cmd.command_buffer, "Light pass", { 0, 0, 1 });
        m_light_image.transition_layout(cmd,
                                        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                        VK_PIPELINE_STAGE_2_NONE,
                                        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                        VK_ACCESS_2_NONE,
                                        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);

        vkCmdBindDescriptorSets(cmd.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_light_pipeline_layout, 0, 1, m_scene.get_scene_descriptor().get_descriptor_set(cmd), 0, nullptr);
        vkCmdBindDescriptorSets(cmd.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_light_pipeline_layout, 1, 1, m_texture_binding.get_descriptor_set(cmd), 0, nullptr);
        vkCmdBindDescriptorSets(cmd.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_light_pipeline_layout, 2, 1, m_scene.get_light_descriptor().get_descriptor_set(cmd), 0, nullptr);

        RenderInfoBuilderOut render_color_info;

        RenderInfoBuilder()
            .add_color(m_light_image.get_view(cmd), VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE)
            .set_size(m_light_image.get_size())
            .build(render_color_info);

        vkCmdBeginRendering(cmd.command_buffer, &render_color_info.rendering_info);

        vkCmdBindPipeline(cmd.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_light_pipeline);
        vkCmdDraw(cmd.command_buffer, 3, 1, 0, 0);

        vkCmdEndRendering(cmd.command_buffer);

        m_light_image.transition_layout(cmd,
                                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                        VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                                        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                                        VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
        debugger::end_debug_label(cmd.command_buffer);
    }
} // namespace pvp