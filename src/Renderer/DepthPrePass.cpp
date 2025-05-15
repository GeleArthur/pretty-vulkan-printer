#include "DepthPrePass.h"

#include "RenderInfoBuilder.h"
#include "Swapchain.h"

#include <Context/Device.h>
#include <DescriptorSets/DescriptorSetBuilder.h>
#include <GraphicsPipeline/GraphicsPipelineBuilder.h>
#include <GraphicsPipeline/PipelineLayoutBuilder.h>
#include <Image/ImageBuilder.h>

namespace pvp
{
    DepthPrePass::DepthPrePass(const Context& context, const PvpScene& scene)
        : m_context{ context }
        , m_scene{ scene }
    {
        create_images();
        build_pipelines();
    }
    void DepthPrePass::draw(VkCommandBuffer cmd)
    {
        m_depth_image.transition_layout(cmd, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout, 0, 1, &m_scene_binding.sets[0], 0, nullptr);

        const auto depth_info = RenderInfoBuilder()
                                    .set_depth(&m_depth_image)
                                    .set_size(m_depth_image.get_size())
                                    .build();

        vkCmdBeginRendering(cmd, &depth_info.rendering_info);
        {
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
            for (const Model& model : m_scene.get_models())
            {
                VkDeviceSize offset{ 0 };
                vkCmdBindVertexBuffers(cmd, 0, 1, &model.vertex_data.get_buffer(), &offset);
                vkCmdBindIndexBuffer(cmd, model.index_data.get_buffer(), 0, VK_INDEX_TYPE_UINT32);
                vkCmdPushConstants(cmd, m_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &model.transform);
                vkCmdDrawIndexed(cmd, model.index_count, 1, 0, 0, 0);
            }
        }
        vkCmdEndRendering(cmd);

        m_depth_image.transition_layout(cmd,
                                        VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                                        VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
                                        VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
                                        VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                                        VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT);
    }
    void DepthPrePass::build_pipelines()
    {
        m_scene_binding = DescriptorSetBuilder()
                              .set_layout(m_context.descriptor_creator->get_layout(0))
                              .bind_buffer(0, *m_scene.get_scene_globals())
                              .build(m_context.device->get_device(), *m_context.descriptor_creator);

        PipelineLayoutBuilder()
            .add_descriptor_layout(m_context.descriptor_creator->get_layout(0))
            .add_push_constant_range(VkPushConstantRange{ VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4) })
            .build(m_context.device->get_device(), m_pipeline_layout);
        m_destructor_queue.add_to_queue([&] { vkDestroyPipelineLayout(m_context.device->get_device(), m_pipeline_layout, nullptr); });

        GraphicsPipelineBuilder()
            .add_shader("shaders/gpass.vert.spv", VK_SHADER_STAGE_VERTEX_BIT)
            .set_depth_format(VK_FORMAT_D32_SFLOAT)
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
            .set_size(m_context.swapchain->get_swapchain_extent())
            .set_usage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
            .build(m_context.device->get_device(), m_context.allocator->get_allocator(), m_depth_image);
        m_destructor_queue.add_to_queue([&] { m_depth_image.destroy(m_context); });
    }
} // namespace pvp