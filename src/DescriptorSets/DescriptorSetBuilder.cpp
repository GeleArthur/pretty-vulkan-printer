#include "DescriptorSetBuilder.h"

#include <array>
#include <globalconst.h>
#include <span>
#include <stdexcept>
#include <UniformBuffers/UniformBuffer.h>
#include <vulkan/vulkan.h>

namespace pvp
{
    DescriptorSetBuilder::DescriptorSetBuilder()
    {
    }

    DescriptorSetBuilder& DescriptorSetBuilder::set_layout(VkDescriptorSetLayout& layout)
    {
        m_descriptor_layout = &layout;
        return *this;
    }

    DescriptorSetBuilder& DescriptorSetBuilder::bind_image(uint32_t binding, const Image& image, const Sampler& sampler, VkImageLayout layout)
    {
        m_images.push_back({ binding, image, sampler, layout });
        return *this;
    }

    DescriptorSets DescriptorSetBuilder::build(const VkDevice device, const DescriptorPool& pool) const
    {
        DescriptorSets descriptor;

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            VkDescriptorSetAllocateInfo alloc_info{};
            alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            alloc_info.descriptorPool = pool.get_handle();
            alloc_info.descriptorSetCount = 1;
            alloc_info.pSetLayouts = m_descriptor_layout;

            if (vkAllocateDescriptorSets(device, &alloc_info, &descriptor.sets[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to allocate descriptor sets!");
            }

            for (const auto& buffer : m_buffers)
            {
                VkDescriptorBufferInfo buffer_info{};
                buffer_info.buffer = std::get<1>(buffer).get()[i].get_buffer();
                buffer_info.offset = 0;
                buffer_info.range = std::get<1>(buffer).get()[i].get_allocation_info().size;

                VkWriteDescriptorSet write{};
                write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write.dstSet = descriptor.sets[i];
                write.dstBinding = std::get<0>(buffer);
                write.dstArrayElement = 0;
                write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                write.descriptorCount = 1;
                write.pBufferInfo = &buffer_info;

                vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
            }

            for (const auto& image : m_images)
            {
                VkDescriptorImageInfo image_info{};
                image_info.imageView = std::get<1>(image).get().get_view();
                image_info.sampler = std::get<2>(image).get().handle;
                image_info.imageLayout = std::get<3>(image) == VK_IMAGE_LAYOUT_MAX_ENUM ? std::get<1>(image).get().get_layout() : std::get<3>(image);

                VkWriteDescriptorSet write{};
                write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write.dstSet = descriptor.sets[i];
                write.dstBinding = std::get<0>(image);
                write.dstArrayElement = 0;
                write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                write.descriptorCount = 1;
                write.pImageInfo = &image_info;

                vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
            }
        }

        return descriptor;
    }
} // namespace pvp
