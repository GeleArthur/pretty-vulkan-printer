#include "DescriptorSetBuilder.h"

#include <array>
#include <globalconst.h>
#include <span>
#include <stdexcept>
#include <Context/Device.h>
#include <UniformBuffers/UniformBuffer.h>
#include <tracy/Tracy.hpp>
#include <vulkan/vulkan.h>

namespace pvp
{
    DescriptorSetBuilder& DescriptorSetBuilder::set_layout(VkDescriptorSetLayout layout)
    {
        m_descriptor_layout = layout;
        return *this;
    }
    DescriptorSetBuilder& DescriptorSetBuilder::bind_uniform_buffer(uint32_t binding, const UniformBuffer& buffer)
    {
        m_uniform_buffers.emplace_back(binding, &buffer);
        return *this;
    }
    DescriptorSetBuilder& DescriptorSetBuilder::bind_buffer_ssbo(uint32_t binding, const Buffer& buffer)
    {
        m_buffers_ssbo.emplace_back(binding, buffer);
        return *this;
    }
    DescriptorSetBuilder& DescriptorSetBuilder::bind_image(uint32_t binding, Image& image, VkImageLayout layout)
    {
        m_images.emplace_back(binding, &image, layout);
        return *this;
    }
    DescriptorSetBuilder& DescriptorSetBuilder::bind_image_array(uint32_t binding, const std::vector<StaticImage>& image_array)
    {
        m_image_array = ImageArrayInfo(binding, &image_array);
        return *this;
    }
    DescriptorSetBuilder& DescriptorSetBuilder::bind_sampler(uint32_t binding, const Sampler& sampler)
    {
        m_samplers.emplace_back(binding, &sampler);
        return *this;
    }

    void DescriptorSetBuilder::build(const Context& context, DescriptorSets& descriptor) const
    {
        ZoneScoped;
        descriptor.m_context = &context;

        int frames = m_is_dynamic ? max_frames_in_flight : 1;

        for (int frame_index = 0; frame_index < frames; ++frame_index)
        {
            ZoneScoped;
            VkDescriptorSetAllocateInfo alloc_info{};
            alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            alloc_info.descriptorPool = context.descriptor_creator->get_pool();
            alloc_info.descriptorSetCount = 1;
            alloc_info.pSetLayouts = &m_descriptor_layout;

            // Bindless
            unsigned int                                       image_count{};
            VkDescriptorSetVariableDescriptorCountAllocateInfo variable_count{};

            if (std::get<1>(m_image_array) != nullptr)
            {
                image_count = std::get<1>(m_image_array)->size();

                variable_count = VkDescriptorSetVariableDescriptorCountAllocateInfo{
                    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
                    .descriptorSetCount = 1,
                    .pDescriptorCounts = &image_count
                };
                alloc_info.pNext = &variable_count;
            }

            if (VkResult error = vkAllocateDescriptorSets(context.device->get_device(), &alloc_info, &descriptor.m_sets[frame_index]); error != VK_SUCCESS)
            {
                throw std::runtime_error("failed to allocate descriptor sets");
            }
            for (const BufferInfo& buffer : m_uniform_buffers)
            {
                VkDescriptorBufferInfo buffer_info{};
                buffer_info.buffer = std::get<1>(buffer)->get_buffer(frame_index).get_buffer();
                buffer_info.offset = 0;
                buffer_info.range = std::get<1>(buffer)->get_buffer(frame_index).get_size();

                VkWriteDescriptorSet write{};
                write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write.dstSet = descriptor.m_sets[frame_index];
                write.dstBinding = std::get<0>(buffer);
                write.dstArrayElement = 0;
                write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                write.descriptorCount = 1;
                write.pBufferInfo = &buffer_info;

                vkUpdateDescriptorSets(context.device->get_device(), 1, &write, 0, nullptr);
            }

            for (const ImageInfo& image : m_images)
            {
                VkDescriptorImageInfo image_info{};
                image_info.imageView = std::get<1>(image)->get_view(frame_index);
                image_info.imageLayout = std::get<2>(image) == VK_IMAGE_LAYOUT_MAX_ENUM ?
                    std::get<1>(image)->get_layout(frame_index) :
                    std::get<2>(image);

                VkWriteDescriptorSet write{};
                write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write.dstSet = descriptor.m_sets[frame_index];
                write.dstBinding = std::get<0>(image);
                write.dstArrayElement = 0;
                write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                write.descriptorCount = 1;
                write.pImageInfo = &image_info;

                // TODO: Not happy with this
                ImageBinding binding{
                    .binding = std::get<0>(image),
                    .layout = image_info.imageLayout,
                    .image = std::get<1>(image),
                    .set = frame_index
                };

                EventListener<>& listener = descriptor.m_images.emplace_back(EventListener<>{ [&descriptor, binding] { descriptor.reconnect_image(binding); } });
                std::get<1>(image)->get_image_invalid().add_listener(&listener);

                vkUpdateDescriptorSets(context.device->get_device(), 1, &write, 0, nullptr);
            }

            if (std::get<1>(m_image_array) != nullptr)
            {
                for (int j = 0; j < std::get<1>(m_image_array)->size(); ++j)
                {
                    VkDescriptorImageInfo image_info{};
                    image_info.imageView = (*std::get<1>(m_image_array))[j].get_view();
                    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                    VkWriteDescriptorSet write{};
                    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    write.dstSet = descriptor.m_sets[frame_index];
                    write.dstBinding = std::get<0>(m_image_array);
                    write.dstArrayElement = j;
                    write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                    write.descriptorCount = 1;
                    write.pImageInfo = &image_info;

                    vkUpdateDescriptorSets(context.device->get_device(), 1, &write, 0, nullptr);
                }
            }

            for (const SamplerInfo& sampler : m_samplers)
            {
                VkDescriptorImageInfo sampler_info{};
                sampler_info.sampler = std::get<1>(sampler)->handle;

                VkWriteDescriptorSet write{};
                write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write.dstSet = descriptor.m_sets[frame_index];
                write.dstBinding = std::get<0>(sampler);
                write.dstArrayElement = 0;
                write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
                write.descriptorCount = 1;
                write.pImageInfo = &sampler_info;

                vkUpdateDescriptorSets(context.device->get_device(), 1, &write, 0, nullptr);
            }

            for (const auto& buffer : m_buffers_ssbo)
            {
                VkDescriptorBufferInfo buffer_info{};
                buffer_info.buffer = std::get<1>(buffer).get_buffer();
                buffer_info.offset = 0;
                buffer_info.range = std::get<1>(buffer).get_size();

                VkWriteDescriptorSet write{};
                write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write.dstSet = descriptor.m_sets[frame_index];
                write.dstBinding = std::get<0>(buffer);
                write.dstArrayElement = 0;
                write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                write.descriptorCount = 1;
                write.pBufferInfo = &buffer_info;

                vkUpdateDescriptorSets(context.device->get_device(), 1, &write, 0, nullptr);
            }
        }
    }
} // namespace pvp
