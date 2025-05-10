#include "GBuffer.h"

#include <array>
#include <PVPDescriptorSets/DescriptorLayoutBuilder.h>
#include <PVPDescriptorSets/DescriptorSetBuilder.h>
#include <PVPGraphicsPipeline/GraphicsPipelineBuilder.h>
#include <PVPGraphicsPipeline/PipelineLayoutBuilder.h>
#include <PVPGraphicsPipeline/Vertex.h>
#include <PVPImage/ImageBuilder.h>
#include <Scene/PVPScene.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

pvp::GBuffer::GBuffer(const Context& context, const PvpScene& scene, const ImageInfo& image_info)
    : m_context(context)
    , m_scene(scene)
    , m_image_info{ image_info }
    , m_camera_uniform{ context.allocator->get_allocator() }
{
    build_pipelines();
    create_images();
}

void pvp::GBuffer::build_pipelines()
{
    vk::DescriptorSetLayout layout;
    DescriptorLayoutBuilder()
        .add_binding(vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex)
        .build(vk::Device(m_context.device->get_device()), layout);
    m_destructor_queue.add_to_queue([&, layout] { vkDestroyDescriptorSetLayout(m_context.device->get_device(), layout, nullptr); });

    float               time = 0;
    ModelCameraViewData ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(m_image_info.image_size.width) / static_cast<float>(m_image_info.image_size.height), 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;

    m_camera_uniform.update(0, ubo);

    m_descriptor_binding = DescriptorSetBuilder()
                               .bind_buffer(0, m_camera_uniform)
                               .set_layout(layout)
                               .build(m_context.device->get_device(), *m_context.descriptor_pool);

    PipelineLayoutBuilder()
        .add_descriptor_layout(layout)
        .build(m_context.device->get_device(), m_albedo_pipeline_layout);
    m_destructor_queue.add_to_queue([&] { vkDestroyPipelineLayout(m_context.device->get_device(), m_albedo_pipeline_layout, nullptr); });

    GraphicsPipelineBuilder()
        .add_shader("shaders/gbuffer-albedo.vert.spv", VK_SHADER_STAGE_VERTEX_BIT)
        .add_shader("shaders/gbuffer-albedo.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
        .set_color_format(std::array{ m_image_info.color_format })
        .set_pipeline_layout(m_albedo_pipeline_layout)
        .set_input_attribute_description(Vertex::get_attribute_descriptions())
        .set_input_binding_description(Vertex::get_binding_description())
        .build(*m_context.device, m_albedo_pipeline);
    m_destructor_queue.add_to_queue([&] { vkDestroyPipeline(m_context.device->get_device(), m_albedo_pipeline, nullptr); });

    GraphicsPipelineBuilder()
        .add_shader("shaders/gbuffer-normal.vert.spv", VK_SHADER_STAGE_VERTEX_BIT)
        .add_shader("shaders/gbuffer-normal.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
        .set_color_format(std::array{ m_image_info.color_format })
        .set_pipeline_layout(m_albedo_pipeline_layout)
        .set_input_attribute_description(Vertex::get_attribute_descriptions())
        .set_input_binding_description(Vertex::get_binding_description())
        .build(*m_context.device, m_normal_pipeline);
    m_destructor_queue.add_to_queue([&] { vkDestroyPipeline(m_context.device->get_device(), m_normal_pipeline, nullptr); });
}

void pvp::GBuffer::create_images()
{
    ImageBuilder()
        .set_format(m_image_info.color_format)
        .set_aspect_flags(VK_IMAGE_ASPECT_COLOR_BIT)
        .set_memory_usage(VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE)
        .set_size(m_image_info.image_size)
        .set_usage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
        .build(m_context.device->get_device(), m_context.allocator->get_allocator(), m_albedo_image);
    m_destructor_queue.add_to_queue([&] { m_albedo_image.destroy(m_context); });

    ImageBuilder()
        .set_format(VK_FORMAT_R8G8B8A8_UNORM)
        .set_aspect_flags(VK_IMAGE_ASPECT_COLOR_BIT)
        .set_memory_usage(VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE)
        .set_size(m_image_info.image_size)
        .set_usage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
        .build(m_context.device->get_device(), m_context.allocator->get_allocator(), m_normal_image);
    m_destructor_queue.add_to_queue([&] { m_normal_image.destroy(m_context); });
}

void pvp::GBuffer::draw(VkCommandBuffer cmd)
{
    constexpr VkClearValue clear_values{ 0.2f, 0.2f, 0.2f, 1.0f };

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

    VkRenderingAttachmentInfo color_info{
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = m_albedo_image.get_view(),
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = clear_values
    };

    VkRenderingAttachmentInfo normal_info{
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = m_albedo_image.get_view(),
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = clear_values
    };

    VkRenderingInfo render_color_info{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = VkRect2D{ VkOffset2D{ 0, 0 }, m_albedo_image.get_size() },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_info,
    };

    VkRenderingInfo render_normal_info{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = VkRect2D{ VkOffset2D{ 0, 0 }, m_normal_image.get_size() },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &normal_info,
    };

    vkCmdBindDescriptorSets(cmd,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m_albedo_pipeline_layout,
                            0,
                            1,
                            &m_descriptor_binding.sets[0],
                            0,
                            nullptr);

    vkCmdBeginRendering(cmd, &render_color_info);
    {
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_albedo_pipeline);

        for (const Model& model : m_scene.models)
        {
            VkDeviceSize offset{ 0 };
            vkCmdBindVertexBuffers(cmd, 0, 1, &model.vertex_data.get_buffer(), &offset);
            vkCmdBindIndexBuffer(cmd, model.index_data.get_buffer(), 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(cmd, model.index_count, 1, 0, 0, 0);
        }
    }
    vkCmdEndRendering(cmd);

    vkCmdBeginRendering(cmd, &render_normal_info);
    {
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_normal_pipeline);

        for (const Model& model : m_scene.models)
        {
            VkDeviceSize offset{ 0 };
            vkCmdBindVertexBuffers(cmd, 0, 1, &model.vertex_data.get_buffer(), &offset);
            vkCmdBindIndexBuffer(cmd, model.index_data.get_buffer(), 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(cmd, model.index_count, 1, 0, 0, 0);
        }
    }
    vkCmdEndRendering(cmd);

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
}
