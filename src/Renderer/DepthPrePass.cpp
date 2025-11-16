#include "DepthPrePass.h"

#include "FrameContext.h"
#include "RenderInfoBuilder.h"
#include "Swapchain.h"

#include <Context/Device.h>
#include <Debugger/Debugger.h>
#include <DescriptorSets/DescriptorSetBuilder.h>
#include <GraphicsPipeline/GraphicsPipelineBuilder.h>
#include <GraphicsPipeline/PipelineLayoutBuilder.h>
#include <GraphicsPipeline/Vertex.h>
#include <Image/ImageBuilder.h>
#include <tracy/Tracy.hpp>
#include <tracy/TracyVulkan.hpp>

namespace pvp
{
    DepthPrePass::DepthPrePass(const Context& context, const PvpScene& scene)
        : m_context{ context }
        , m_scene{ scene }
    {
        create_images();
        build_pipelines();
    }

    void DepthPrePass::draw(const FrameContext& cmd)
    {
        ZoneScoped;

        TracyVkZone(m_context.tracy_ctx[cmd.buffer_index], cmd.command_buffer, "DepthPrePass");
        Debugger::start_debug_label(cmd.command_buffer, "Depth pre pass", { 0.7f, 0, 0 });

        ZoneNamedN(transition, "TransitionLayout", true);
        m_depth_image.transition_layout(cmd,
                                        VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                                        VK_PIPELINE_STAGE_2_NONE,
                                        VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
                                        VK_ACCESS_2_NONE,
                                        VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
        ZoneNamedN(bind_texture, "Bind Scene+Texture", true);
        vkCmdBindDescriptorSets(cmd.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout, 0, 1, m_scene.get_scene_descriptor().get_descriptor_set(cmd), 0, nullptr);
        vkCmdBindDescriptorSets(cmd.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout, 1, 1, m_scene.get_textures_descriptor().get_descriptor_set(cmd), 0, nullptr);

        RenderInfoBuilderOut depth_info;
        RenderInfoBuilder()
            .set_depth(m_depth_image.get_view(cmd), VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE)
            .set_size(m_depth_image.get_size())
            .build(depth_info);

        ZoneNamedN(begin_rendering, "begin rendering", true);
        vkCmdBeginRendering(cmd.command_buffer, &depth_info.rendering_info);
        {
            vkCmdBindPipeline(cmd.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
            for (const Model& model : m_scene.get_models())
            {
                ZoneScopedN("Draw");
                VkDeviceSize offset{ 0 };
                vkCmdBindVertexBuffers(cmd.command_buffer, 0, 1, &model.vertex_data.get_buffer(), &offset);
                vkCmdBindIndexBuffer(cmd.command_buffer, model.index_data.get_buffer(), 0, VK_INDEX_TYPE_UINT32);
                vkCmdPushConstants(cmd.command_buffer, m_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(MaterialTransform), &model.material.transform);
                vkCmdDrawIndexed(cmd.command_buffer, model.index_count, 1, 0, 0, 0);
            }
        }
        ZoneNamedN(end_rendering, "end rendering", true);
        vkCmdEndRendering(cmd.command_buffer);

        ZoneNamedN(transition_depth, "transition depth", true);
        m_depth_image.transition_layout(cmd,
                                        VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                                        VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                                        VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
                                        VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                                        VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT);

        Debugger::end_debug_label(cmd.command_buffer);
    }

    void DepthPrePass::build_pipelines()
    {
        PipelineLayoutBuilder()
            .add_descriptor_layout(m_context.descriptor_creator->get_layout(0))
            .add_descriptor_layout(m_context.descriptor_creator->get_layout(1))
            .add_push_constant_range(VkPushConstantRange{ VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(MaterialTransform) })
            .build(m_context.device->get_device(), m_pipeline_layout);
        m_destructor_queue.add_to_queue([&] { vkDestroyPipelineLayout(m_context.device->get_device(), m_pipeline_layout, nullptr); });

        GraphicsPipelineBuilder()
            .add_shader("shaders/depthpass.vert", VK_SHADER_STAGE_VERTEX_BIT)
            .add_shader("shaders/depthpass.frag", VK_SHADER_STAGE_FRAGMENT_BIT)
            .set_depth_format(m_depth_image.get_format())
            .set_pipeline_layout(m_pipeline_layout)
            .set_input_attribute_description(Vertex::get_attribute_descriptions())
            .set_input_binding_description(Vertex::get_binding_description())
            .set_depth_access(VK_TRUE, VK_TRUE)
            .build(*m_context.device, m_pipeline);
        m_destructor_queue.add_to_queue([&] { vkDestroyPipeline(m_context.device->get_device(), m_pipeline, nullptr); });
    }

    void DepthPrePass::create_images()
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
} // namespace pvp
