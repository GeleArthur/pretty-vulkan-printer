#include "LightPass.h"

#include <PVPDescriptorSets/DescriptorLayoutBuilder.h>
#include <PVPGraphicsPipeline/PipelineLayoutBuilder.h>
#include <PVPImage/ImageBuilder.h>
#include <PVPImage/SamplerBuilder.h>
#include <glm/gtx/rotate_vector.hpp>

namespace pvp
{
    LightPass::LightPass(const Context& context, const ImageInfo& image_info, GBuffer& gbuffer)
        : m_context{ context }
        , m_gemotry_pass{ gbuffer }
        , m_image_info{ image_info }
    {
        build_pipelines();
        create_images();
    }
    void LightPass::draw(VkCommandBuffer command_buffer)
    {
        m_descriptor_binding = DescriptorSetBuilder()
                                   .bind_image(0, m_gemotry_pass.get_albedo_image(), m_sampler)
                                   .bind_image(0, m_gemotry_pass.get_normal_image(), m_sampler)
                                   .set_layout(m_desciptor_layout)
                                   .build(m_context.device->get_device(), *m_context.descriptor_pool);
    }
    void LightPass::build_pipelines()
    {
        DescriptorLayoutBuilder()
            .add_binding(vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
            .add_binding(vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
            .build(vk::Device(m_context.device->get_device()), m_desciptor_layout);
        m_destructor_queue.add_to_queue([&] { vkDestroyDescriptorSetLayout(m_context.device->get_device(), m_desciptor_layout, nullptr); });

        SamplerBuilder()
            .set_filter(VK_FILTER_NEAREST)
            .set_address_mode(VK_SAMPLER_ADDRESS_MODE_REPEAT)
            .build(m_context.device->get_device(), m_sampler);

        PipelineLayoutBuilder()
            .add_descriptor_layout(m_desciptor_layout)
            .build(m_context.device->get_device(), m_light_pipeline_layout);
        m_destructor_queue.add_to_queue([&] { vkDestroyPipelineLayout(m_context.device->get_device(), m_light_pipeline_layout, nullptr); });

        GraphicsPipelineBuilder()
            .add_shader("shaders/lightpass.vert.spv", VK_SHADER_STAGE_VERTEX_BIT)
            .add_shader("shaders/lightpass.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
            .set_color_format(std::array{ m_image_info.color_format })
            .set_pipeline_layout(m_light_pipeline_layout)
            // .set_input_attribute_description(Vertex::get_attribute_descriptions())
            // .set_input_binding_description(Vertex::get_binding_description())
            .build(*m_context.device, m_light_pipeline);
        m_destructor_queue.add_to_queue([&] { vkDestroyPipeline(m_context.device->get_device(), m_light_pipeline, nullptr); });
    }
    void LightPass::create_images()
    {
        ImageBuilder()
            .set_format(m_image_info.color_format)
            .set_aspect_flags(VK_IMAGE_ASPECT_COLOR_BIT)
            .set_memory_usage(VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE)
            .set_size(m_image_info.image_size)
            .set_usage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
            .build(m_context.device->get_device(), m_context.allocator->get_allocator(), m_light_image);
        m_destructor_queue.add_to_queue([&] { m_light_image.destroy(m_context); });
    }
} // namespace pvp