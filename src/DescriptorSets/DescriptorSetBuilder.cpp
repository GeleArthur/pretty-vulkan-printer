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
    DescriptorSetBuilder::DescriptorSetBuilder()
    {
    }

    DescriptorSetBuilder& DescriptorSetBuilder::set_layout(VkDescriptorSetLayout layout)
    {
        m_descriptor_layout = layout;
        return *this;
    }

    DescriptorSetBuilder& DescriptorSetBuilder::bind_buffer_ssbo(uint32_t binding, const std::vector<Buffer>& buffer)
    {
        m_buffers_ssbo.emplace_back(binding, buffer);
        return *this;
    }
    DescriptorSetBuilder& DescriptorSetBuilder::bind_image(uint32_t binding, const Image& image, VkImageLayout layout)
    {
        m_images.push_back({ binding, image, layout });
        return *this;
    }
    DescriptorSetBuilder& DescriptorSetBuilder::bind_image_array(uint32_t binding, const std::vector<Image>& image_array)
    {
        m_image_array = std::make_unique<std::tuple<uint32_t, std::vector<Image>>>(binding, image_array);
        return *this;
    }
    DescriptorSetBuilder& DescriptorSetBuilder::bind_sampler(uint32_t binding, const Sampler& sampler)
    {
        m_samplers.push_back({ binding, sampler });
        return *this;
    }

    DescriptorSets DescriptorSetBuilder::build(const Context& context) const
    {
        DescriptorSets descriptor;

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            VkDescriptorSetAllocateInfo alloc_info{};

            std::unique_ptr<unsigned>                                           image_count{};
            std::unique_ptr<VkDescriptorSetVariableDescriptorCountAllocateInfo> variable_count{};

            if (m_image_array != nullptr)
            {
                image_count = std::make_unique<uint32_t>(static_cast<uint32_t>(std::get<1>(*m_image_array).size()));

                variable_count = std::make_unique<VkDescriptorSetVariableDescriptorCountAllocateInfo>(VkDescriptorSetVariableDescriptorCountAllocateInfo{
                    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
                    .descriptorSetCount = 1,
                    .pDescriptorCounts = image_count.get() });
                alloc_info.pNext = variable_count.get();
            }

            alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            alloc_info.descriptorPool = context.descriptor_creator->get_pool();
            alloc_info.descriptorSetCount = 1;
            alloc_info.pSetLayouts = &m_descriptor_layout;

            if (vkAllocateDescriptorSets(context.device->get_device(), &alloc_info, &descriptor.sets[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to allocate descriptor sets!");
            }

            for (const auto& buffer : m_buffers)
            {
                VkDescriptorBufferInfo buffer_info{};
                buffer_info.buffer = std::get<1>(buffer).get()[i].get_buffer();
                buffer_info.offset = 0;
                buffer_info.range = std::get<1>(buffer).get()[i].get_size();

                VkWriteDescriptorSet write{};
                write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write.dstSet = descriptor.sets[i];
                write.dstBinding = std::get<0>(buffer);
                write.dstArrayElement = 0;
                write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                write.descriptorCount = 1;
                write.pBufferInfo = &buffer_info;

                vkUpdateDescriptorSets(context.device->get_device(), 1, &write, 0, nullptr);
            }

            for (const auto& buffer : m_buffers_ssbo)
            {
                VkDescriptorBufferInfo buffer_info{};
                buffer_info.buffer = std::get<1>(buffer).get()[i].get_buffer();
                buffer_info.offset = 0;
                buffer_info.range = std::get<1>(buffer).get()[i].get_size();

                VkWriteDescriptorSet write{};
                write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write.dstSet = descriptor.sets[i];
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
                image_info.imageView = std::get<1>(image).get().get_view();
                image_info.imageLayout = std::get<2>(image) == VK_IMAGE_LAYOUT_MAX_ENUM ? std::get<1>(image).get().get_layout() : std::get<2>(image);

                VkWriteDescriptorSet write{};
                write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write.dstSet = descriptor.sets[i];
                write.dstBinding = std::get<0>(image);
                write.dstArrayElement = 0;
                write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                write.descriptorCount = 1;
                write.pImageInfo = &image_info;

                vkUpdateDescriptorSets(context.device->get_device(), 1, &write, 0, nullptr);
            }

            if (m_image_array != nullptr)
            {
                for (int j = 0; j < std::get<1>(*m_image_array).size(); ++j)
                {
                    VkDescriptorImageInfo image_info{};
                    image_info.imageView = std::get<1>(*m_image_array)[j].get_view();
                    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                    VkWriteDescriptorSet write{};
                    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    write.dstSet = descriptor.sets[i];
                    write.dstBinding = std::get<0>(*m_image_array);
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
                sampler_info.sampler = std::get<1>(sampler).get().handle;

                VkWriteDescriptorSet write{};
                write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write.dstSet = descriptor.sets[i];
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
