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

        DescriptorSetBuilder& bind_uniform_buffer(uint32_t binding, const UniformBuffer& buffer);
        DescriptorSetBuilder& bind_buffer_ssbo(uint32_t binding, const Buffer& buffer);
        DescriptorSetBuilder& bind_image(uint32_t binding, Image& image, VkImageLayout layout = VK_IMAGE_LAYOUT_MAX_ENUM);
        DescriptorSetBuilder& bind_image_array(uint32_t binding, const std::vector<StaticImage>& image_array);
        DescriptorSetBuilder& bind_sampler(uint32_t binding, const Sampler& sampler);

        void build(const Context& context, DescriptorSets& descriptor) const;

    private:
        template<typename... T>
        using DescriptorLocation = std::tuple<uint32_t, T...>;

        using BufferInfo = DescriptorLocation<const UniformBuffer*>;
        using ImageInfo = DescriptorLocation<Image*, VkImageLayout>;
        using ImageArrayInfo = DescriptorLocation<std::vector<StaticImage> const*>;
        using SamplerInfo = DescriptorLocation<const Sampler*>;
        using SSBOInfo = DescriptorLocation<const Buffer&>;

        VkDescriptorSetLayout    m_descriptor_layout{};
        std::vector<BufferInfo>  m_uniform_buffers;
        std::vector<SSBOInfo>    m_buffers_ssbo;
        std::vector<ImageInfo>   m_images;
        std::vector<SamplerInfo> m_samplers;
        ImageArrayInfo           m_image_array{};
        bool                     m_is_dynamic{ true };
    };
} // namespace pvp
