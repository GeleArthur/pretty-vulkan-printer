#include "DescriptorSetBuilder.h"

#include <array>
#include <globalconst.h>
#include <span>
#include <stdexcept>
#include <Context/Device.h>
#include <UniformBuffers/UniformBuffer.h>
#include <vulkan/vulkan.h>

namespace pvp
{
    DescriptorSetBuilder& DescriptorSetBuilder::set_layout(VkDescriptorSetLayout layout)
    {
        m_descriptor_layout = layout;
        return *this;
    }

    DescriptorSetBuilder& DescriptorSetBuilder::bind_buffer_ssbo(uint32_t binding, Buffer& buffer)
    {
        m_buffers_ssbo = BufferInfo(binding, buffer);
        return *this;
    }
    DescriptorSetBuilder& DescriptorSetBuilder::bind_image(uint32_t binding, const Image& image, VkImageLayout layout)
    {
        m_images.emplace_back(binding, &image, layout);
        return *this;
    }
    DescriptorSetBuilder& DescriptorSetBuilder::bind_image_array(uint32_t binding, const std::vector<StaticImage>& image_array)
    {
        m_image_array = std::tuple<uint32_t, std::vector<StaticImage> const*>(binding, &image_array);
        return *this;
    }
    DescriptorSetBuilder& DescriptorSetBuilder::bind_sampler(uint32_t binding, const Sampler& sampler)
    {
        m_samplers.emplace_back(binding, &sampler);
        return *this;
    }

    DescriptorSets DescriptorSetBuilder::build(const Context& context) const
    {
        DescriptorSets descriptor;

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            VkDescriptorSetAllocateInfo alloc_info{};

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

            alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            alloc_info.descriptorPool = context.descriptor_creator->get_pool();
            alloc_info.descriptorSetCount = 1;
            alloc_info.pSetLayouts = &m_descriptor_layout;

            if (vkAllocateDescriptorSets(context.device->get_device(), &alloc_info, &descriptor.m_sets[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to allocate descriptor sets!");
            }

            for (const auto& buffer : m_buffers)
            {
                VkDescriptorBufferInfo buffer_info{};
                buffer_info.buffer = std::get<1>(buffer)[i].get_buffer();
                buffer_info.offset = 0;
                buffer_info.range = std::get<1>(buffer)[i].get_size();

                VkWriteDescriptorSet write{};
                write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write.dstSet = descriptor.m_sets[i];
                write.dstBinding = std::get<0>(buffer);
                write.dstArrayElement = 0;
                write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                write.descriptorCount = 1;
                write.pBufferInfo = &buffer_info;

                vkUpdateDescriptorSets(context.device->get_device(), 1, &write, 0, nullptr);
            }

            for (const std::tuple<uint32_t, const std::vector<Buffer>>& buffer : m_buffers_ssbo)
            {
                VkDescriptorBufferInfo buffer_info{};
                buffer_info.buffer = std::get<1>(buffer)[0].get_buffer();
                buffer_info.offset = 0;
                buffer_info.range = std::get<1>(buffer)[0].get_size();

                VkWriteDescriptorSet write{};
                write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write.dstSet = descriptor.m_sets[0];
                write.dstBinding = std::get<0>(buffer);
                write.dstArrayElement = 0;
                write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                write.descriptorCount = 1;
                write.pBufferInfo = &buffer_info;

                vkUpdateDescriptorSets(context.device->get_device(), 1, &write, 0, nullptr);
            }

            for (const auto& image : m_images)
            {
                VkDescriptorImageInfo image_info{};
                image_info.imageView = std::get<1>(image)->get_view(i);
                image_info.imageLayout = std::get<2>(image) == VK_IMAGE_LAYOUT_MAX_ENUM ? std::get<1>(image)->get_layout(i) : std::get<2>(image);

                VkWriteDescriptorSet write{};
                write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write.dstSet = descriptor.m_sets[i];
                write.dstBinding = std::get<0>(image);
                write.dstArrayElement = 0;
                write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                write.descriptorCount = 1;
                write.pImageInfo = &image_info;

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
                    write.dstSet = descriptor.m_sets[i];
                    write.dstBinding = std::get<0>(m_image_array);
                    write.dstArrayElement = j;
                    write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                    write.descriptorCount = 1;
                    write.pImageInfo = &image_info;

                    vkUpdateDescriptorSets(context.device->get_device(), 1, &write, 0, nullptr);
                }
            }

            for (const auto& sampler : m_samplers)
            {
                VkDescriptorImageInfo sampler_info{};
                sampler_info.sampler = std::get<1>(sampler)->handle;

                VkWriteDescriptorSet write{};
                write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write.dstSet = descriptor.m_sets[i];
                write.dstBinding = std::get<0>(sampler);
                write.dstArrayElement = 0;
                write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
                write.descriptorCount = 1;
                write.pImageInfo = &sampler_info;

                vkUpdateDescriptorSets(context.device->get_device(), 1, &write, 0, nullptr);
            }
        }

        return descriptor;
    }
} // namespace pvp
