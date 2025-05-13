#include "SamplerBuilder.h"

#include <stdexcept>
#include <vulkan/vulkan.h>

#include "Sampler.h"

SamplerBuilder& SamplerBuilder::set_filter(const VkFilter filter)
{
    m_sampler_info.magFilter = filter;
    m_sampler_info.minFilter = filter;
    return *this;
}

SamplerBuilder& SamplerBuilder::set_mipmap(const VkSamplerMipmapMode mode)
{
    m_sampler_info.mipmapMode = mode;
    return *this;
}

SamplerBuilder& SamplerBuilder::set_address_mode(const VkSamplerAddressMode mode)
{
    m_sampler_info.addressModeU = mode;
    m_sampler_info.addressModeV = mode;
    m_sampler_info.addressModeW = mode;
    return *this;
}

void SamplerBuilder::build(VkDevice device, pvp::Sampler& sampler) const
{
    if (vkCreateSampler(device, &m_sampler_info, nullptr, &sampler.handle) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture sampler!");
    }
}
