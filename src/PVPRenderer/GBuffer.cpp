#include "GBuffer.h"

#include <UniformBufferStruct.h>
#include <array>
#include <PVPDescriptorSets/DescriptorLayout.h>
#include <PVPDescriptorSets/DescriptorLayoutBuilder.h>
#include <PVPDescriptorSets/DescriptorPool.h>
#include <PVPGraphicsPipeline/GraphicsPipelineBuilder.h>
#include <PVPGraphicsPipeline/PipelineLayoutBuilder.h>
#include <PVPGraphicsPipeline/Vertex.h>
#include <PVPUniformBuffers/UniformBuffer.h>

void pvp::GBuffer::draw(VkCommandBuffer graphics_command)
{
}

void pvp::GBuffer::build_pipelines(const RenderingContext& rendering_context)
{
    vk::DescriptorSetLayout layout;
    DescriptorLayoutBuilder()
        .add_binding(vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex)
        .build(vk::Device(m_context.device->get_device()), layout);

    // m_descriptor_pool = DescriptorPool(m_device.get_device(), { { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2 } }, 2);

    // m_uniform_buffer = new UniformBuffer<ModelCameraViewData>(m_context.allocator->get_allocator());

    PipelineLayoutBuilder()
        .add_descriptor_layout(layout)
        .build(m_context.device->get_device(), m_albedo_pipeline_layout);

    GraphicsPipelineBuilder()
        .add_shader("shaders/gbuffer-albedo.vert.spr", VK_SHADER_STAGE_VERTEX_BIT)
        .add_shader("shaders/gbuffer-albedo.frag.spr", VK_SHADER_STAGE_FRAGMENT_BIT)
        .set_color_format(std::array{ rendering_context.color_format })
        .set_pipeline_layout(m_albedo_pipeline_layout)
        .set_input_attribute_description(Vertex::get_attribute_descriptions())
        .set_input_binding_description(Vertex::get_binding_description())
        .build(*m_context.device, m_albedo_pipeline);
}

void pvp::GBuffer::draw_albedo(const RenderingContext& rendering_context)
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

    vkCmdBindPipeline(rendering_context.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_albedo_pipeline);

    // VkBuffer     vertex_buffers[] = { rendering_context.vertex_buffer.get_buffer() };
    // VkDeviceSize offsets[] = { 0 };
    // vkCmdBindVertexBuffers(rendering_context.command_buffer, 0, 1, vertex_buffers, offsets);
    //
    // vkCmdBindDescriptorSets(rendering_context.command_buffer,
    //                         VK_PIPELINE_BIND_POINT_GRAPHICS,
    //                         m_albedo_pipeline_layout,
    //                         0,
    //                         1,
    //                         &m_descriptors.sets[m_double_buffer_frame],
    //                         0,
    //                         nullptr);
    //
    // vkCmdBindIndexBuffer(graphics_command, m_index_buffer.get_buffer(), 0, VK_INDEX_TYPE_UINT32);
    // vkCmdDrawIndexed(graphics_command, m_model.indices.size(), 1, 0, 0, 0);
}
