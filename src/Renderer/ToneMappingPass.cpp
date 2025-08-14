#include "ToneMappingPass.h"

#include "FrameContext.h"
#include "LightPass.h"
#include "RenderInfoBuilder.h"
#include "Swapchain.h"

#include <Context/Device.h>
#include <Debugger/Debugger.h>
#include <DescriptorSets/DescriptorLayoutCreator.h>
#include <DescriptorSets/DescriptorLayoutBuilder.h>
#include <DescriptorSets/DescriptorSetBuilder.h>
#include <GraphicsPipeline/GraphicsPipelineBuilder.h>
#include <GraphicsPipeline/PipelineLayoutBuilder.h>
#include <Image/ImageBuilder.h>
#include <Image/SamplerBuilder.h>
#include <tracy/Tracy.hpp>
#include <tracy/TracyVulkan.hpp>

namespace pvp
{
    ToneMappingPass::ToneMappingPass(const Context& context, LightPass& light_pass)
        : m_context{ context }
        , m_light_pass{ light_pass }
    {
        ZoneScoped;
        create_images();
        build_pipelines();
    }

    void ToneMappingPass::draw(const FrameContext& cmd)
    {
        ZoneScoped;
        TracyVkZone(m_context.tracy_ctx[cmd.buffer_index], cmd.command_buffer, "ToneMapping");
        Debugger::start_debug_label(cmd.command_buffer, "tone mapping", { 0.8, 0.8f, 0.0f });
        m_tone_texture.transition_layout(cmd,
                                         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                         VK_PIPELINE_STAGE_2_NONE,
                                         VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                         VK_ACCESS_2_NONE,
                                         VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);

        vkCmdBindDescriptorSets(cmd.command_buffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_tone_pipeline_layout,
                                0,
                                1,
                                m_tone_binding.get_descriptor_set(cmd),
                                0,
                                nullptr);

        RenderInfoBuilderOut render_color_info;
        RenderInfoBuilder()
            .add_color(m_tone_texture.get_view(cmd), VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE)
            .set_size(m_tone_texture.get_size())
            .build(render_color_info);

        vkCmdBeginRendering(cmd.command_buffer, &render_color_info.rendering_info);
        {
            vkCmdBindPipeline(cmd.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_tone_pipeline);
            vkCmdDraw(cmd.command_buffer, 3, 1, 0, 0);
        }
        vkCmdEndRendering(cmd.command_buffer);

        m_tone_texture.transition_layout(cmd,
                                         VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                         VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                         VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                                         VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                                         VK_ACCESS_2_TRANSFER_READ_BIT);
        Debugger::end_debug_label(cmd.command_buffer);
    }

    void ToneMappingPass::build_pipelines()
    {
        ZoneScoped;
        m_context.descriptor_creator->create_layout()
            .add_binding(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT)
            .build(12);

        m_destructor_queue.add_to_queue([&] { m_context.descriptor_creator->remove_layout(12); });

        DescriptorSetBuilder()
            .bind_image(0, m_light_pass.get_light_image(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
            .set_layout(m_context.descriptor_creator->get_layout(12))
            .build(m_context, m_tone_binding);

        PipelineLayoutBuilder()
            .add_descriptor_layout(m_context.descriptor_creator->get_layout(12))
            .build(m_context.device->get_device(), m_tone_pipeline_layout);
        m_destructor_queue.add_to_queue([&] { vkDestroyPipelineLayout(m_context.device->get_device(), m_tone_pipeline_layout, nullptr); });

        GraphicsPipelineBuilder()
            .add_shader("shaders/lightpass.vert", VK_SHADER_STAGE_VERTEX_BIT)
            .add_shader("shaders/tonemapping.frag", VK_SHADER_STAGE_FRAGMENT_BIT)
            .set_color_format(std::array{ m_tone_texture.get_format() })
            .set_pipeline_layout(m_tone_pipeline_layout)
            .build(*m_context.device, m_tone_pipeline);
        m_destructor_queue.add_to_queue([&] { vkDestroyPipeline(m_context.device->get_device(), m_tone_pipeline, nullptr); });
    }

    void ToneMappingPass::create_images()
    {
        ZoneScoped;
        ImageBuilder()
            .set_format(VK_FORMAT_R32G32B32A32_SFLOAT)
            .set_aspect_flags(VK_IMAGE_ASPECT_COLOR_BIT)
            .set_memory_usage(VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE)
            .set_screen_size_auto_update(true)
            .set_usage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
            .build(m_context, m_tone_texture);
        m_destructor_queue.add_to_queue([&] { m_tone_texture.destroy(m_context); });
    }
} // namespace pvp