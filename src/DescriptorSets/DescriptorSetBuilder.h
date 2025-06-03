#pragma once
#include <span>
#include <Buffer/Buffer.h>
#include <Image/Image.h>
#include <vulkan/vulkan.hpp>

#include "DescriptorCreator.h"
#include "DescriptorSets.h"

#include <UniformBuffers/UniformBuffer.h>

namespace pvp
{
    class DescriptorSetBuilder final
    {
    public:
        explicit DescriptorSetBuilder();
        DescriptorSetBuilder& set_layout(VkDescriptorSetLayout layout);

        template<typename T>
        DescriptorSetBuilder& bind_buffer(uint32_t binding, const UniformBuffer<T>& buffer);
        DescriptorSetBuilder& bind_image(uint32_t binding, const Image& image, VkImageLayout layout = VK_IMAGE_LAYOUT_MAX_ENUM);
        DescriptorSetBuilder& bind_image_array(uint32_t binding, const std::vector<Image>& image_array);
        DescriptorSetBuilder& bind_sampler(uint32_t binding, const Sampler& sampler);

        DescriptorSets build(const Context& context) const;

    private:
        using image_info = std::tuple<uint32_t, std::reference_wrapper<const Image>, VkImageLayout>;
        using buffer_info = std::tuple<uint32_t, std::reference_wrapper<const std::vector<Buffer>>>;
        using image_array_info = std::unique_ptr<std::tuple<uint32_t, std::vector<Image>>>;
        using sampler_info = std::tuple<uint32_t, std::reference_wrapper<const Sampler>>;

        VkDescriptorSetLayout     m_descriptor_layout;
        std::vector<buffer_info>  m_buffers;
        std::vector<image_info>   m_images;
        std::vector<sampler_info> m_samplers;
        image_array_info          m_image_array{};
    };

    template<typename T>
    DescriptorSetBuilder& DescriptorSetBuilder::bind_buffer(uint32_t binding, const UniformBuffer<T>& buffer)
    {
        m_buffers.emplace_back(binding, buffer.get_buffers());
        return *this;
    }
} // namespace pvp
