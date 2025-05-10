#include "GBuffer.h"

#include <array>
#include <PVPDescriptorSets/DescriptorLayoutBuilder.h>
#include <PVPDescriptorSets/DescriptorPool.h>
#include <PVPDescriptorSets/DescriptorSetBuilder.h>
#include <PVPGraphicsPipeline/GraphicsPipelineBuilder.h>
#include <PVPGraphicsPipeline/PipelineLayoutBuilder.h>
#include <PVPGraphicsPipeline/Vertex.h>
#include <Scene/PVPScene.h>

pvp::GBuffer::GBuffer(const Context& context, const PvpScene& scene, const ImageInfo& image_info)
    : m_context(context)
    , m_scene(scene)
    , m_image_info{ image_info }
    , m_camera_uniform{ context.allocator->get_allocator() }
{
    build_pipelines();
}

void pvp::GBuffer::draw()
{
}

void pvp::GBuffer::build_pipelines()
{
    vk::DescriptorSetLayout layout;
    DescriptorLayoutBuilder()
        .add_binding(vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex)
        .build(vk::Device(m_context.device->get_device()), layout);

    m_camera_uniform = UniformBuffer<ModelCameraViewData>(m_context.allocator->get_allocator());

    PipelineLayoutBuilder()
        .add_descriptor_layout(layout)
        .build(m_context.device->get_device(), m_albedo_pipeline_layout);

    GraphicsPipelineBuilder()
        .add_shader("shaders/gbuffer-albedo.vert.spr", VK_SHADER_STAGE_VERTEX_BIT)
        .add_shader("shaders/gbuffer-albedo.frag.spr", VK_SHADER_STAGE_FRAGMENT_BIT)
        .set_color_format(std::array{ m_image_info.color_format })
        .set_pipeline_layout(m_albedo_pipeline_layout)
        .set_input_attribute_description(Vertex::get_attribute_descriptions())
        .set_input_binding_description(Vertex::get_binding_description())
        .build(*m_context.device, m_albedo_pipeline);

    // m_descriptors = DescriptorSetBuilder()
    //                     .set_layout(VkDescriptorSetLayout(layout.))
    //                     .bind_buffer(0, *m_uniform_buffer)
    //                     // .bind_image(1, m_texture, m_sampler)
    //                     .build(m_device.get_device(), m_descriptor_pool);
}

void pvp::GBuffer::draw_albedo(VkCommandBuffer cmd)
{
    constexpr VkClearValue clear_values{ 0.2f, 0.2f, 0.2f, 1.0f };

    VkRenderingAttachmentInfo color_info{
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = m_albedo_image.get_view(),
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = clear_values
    };

    VkRenderingInfo render_info{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = VkRect2D{ VkOffset2D{ 0, 0 }, m_albedo_image.get_size() },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_info,
    };
    vkCmdBeginRendering(cmd, &render_info);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_albedo_pipeline);

    VkDeviceSize offsets[] = { 0 };

    // vkCmdBindDescriptorSets(cmd,
    //                         VK_PIPELINE_BIND_POINT_GRAPHICS,
    //                         m_albedo_pipeline_layout,
    //                         0,
    //                         1,
    //                         &m_camera_uniform.get_buffer(0),
    //                         0,
    //                         nullptr);

    for (const Model& model : m_scene.models)
    {
        vkCmdBindVertexBuffers(cmd, 0, 1, &model.vertex_data.get_buffer(), offsets);
        vkCmdBindIndexBuffer(cmd, model.index_data.get_buffer(), 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(cmd, model.index_count, 1, 0, 0, 0);
    }

    vkCmdEndRendering(cmd);
}
