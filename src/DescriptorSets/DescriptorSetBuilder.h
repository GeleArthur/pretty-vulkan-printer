#pragma once
#include <span>
#include <Buffer/Buffer.h>
#include <Image/Image.h>
#include <vulkan/vulkan.hpp>

#include "DescriptorPool.h"
#include "DescriptorSets.h"

#include <UniformBuffers/UniformBuffer.h>

namespace pvp
{
    class DescriptorSetBuilder final
    {
    public:
        explicit DescriptorSetBuilder();
        DescriptorSetBuilder& set_layout(VkDescriptorSetLayout& layout);

        template<typename T>
        DescriptorSetBuilder& bind_buffer(uint32_t binding, const UniformBuffer<T>& buffer);
        DescriptorSetBuilder& bind_image(uint32_t binding, const Image& image, const Sampler& sampler, VkImageLayout layout = VK_IMAGE_LAYOUT_MAX_ENUM);

        DescriptorSets build(const VkDevice device, const DescriptorPool& pool) const;

    private:
        using image_info = std::tuple<uint32_t, std::reference_wrapper<const Image>, std::reference_wrapper<const Sampler>, VkImageLayout>;
        using buffer_info = std::tuple<uint32_t, std::reference_wrapper<const std::vector<Buffer>>>;
        VkDescriptorSetLayout*   m_descriptor_layout;
        std::vector<buffer_info> m_buffers;
        std::vector<image_info>  m_images;
    };

    template<typename T>
    DescriptorSetBuilder& DescriptorSetBuilder::bind_buffer(uint32_t binding, const UniformBuffer<T>& buffer)
    {
        m_buffers.push_back({ binding, buffer.get_buffers() });
        return *this;
    }
} // namespace pvp
