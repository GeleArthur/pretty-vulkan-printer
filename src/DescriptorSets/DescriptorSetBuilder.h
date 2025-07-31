#pragma once
#include <span>
#include <Buffer/Buffer.h>
#include <Image/Image.h>
#include <vulkan/vulkan.hpp>

#include "DescriptorLayoutCreator.h"
#include "DescriptorSets.h"

#include <Image/StaticImage.h>
#include <UniformBuffers/UniformBuffer.h>

namespace pvp
{
    class DescriptorSetBuilder final
    {
    public:
        DescriptorSetBuilder& set_layout(VkDescriptorSetLayout layout);

        template<typename T>
        DescriptorSetBuilder& bind_buffer(uint32_t binding, const UniformBuffer<T>& buffer);
        DescriptorSetBuilder& bind_buffer_ssbo(uint32_t binding, const std::vector<Buffer>& buffer);
        DescriptorSetBuilder& bind_image(uint32_t binding, const Image& image, VkImageLayout layout = VK_IMAGE_LAYOUT_MAX_ENUM);
        DescriptorSetBuilder& bind_image_array(uint32_t binding, const std::vector<StaticImage>& image_array);
        DescriptorSetBuilder& bind_sampler(uint32_t binding, const Sampler& sampler);

        DescriptorSets build(const Context& context) const;

    private:
        template<typename... T>
        using DescriptorLocation = std::tuple<uint32_t, T...>;

        using ImageInfo = DescriptorLocation<const Image*, VkImageLayout>;
        using BufferInfo = DescriptorLocation<const std::vector<Buffer>>;
        using ImageArrayInfo = DescriptorLocation<std::vector<StaticImage> const*>;
        using SamplerInfo = DescriptorLocation<const Sampler*>;

        VkDescriptorSetLayout       m_descriptor_layout{};
        std::vector<BufferInfo>     m_buffers;
        DescriptorLocation<Buffer*> m_buffers_ssbo;
        std::vector<ImageInfo>      m_images;
        std::vector<SamplerInfo>    m_samplers;
        ImageArrayInfo              m_image_array{};
    };

    template<typename T>
    DescriptorSetBuilder& DescriptorSetBuilder::bind_buffer(uint32_t binding, const UniformBuffer<T>& buffer)
    {
        m_buffers.emplace_back(binding, buffer.get_buffers());
        return *this;
    }
} // namespace pvp
