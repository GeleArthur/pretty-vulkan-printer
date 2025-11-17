#include "SamplerBuilder.h"

#include <stdexcept>
#include <vulkan/vulkan.h>

#include "Sampler.h"

#include <Context/Context.h>
#include <Context/Device.h>
#include <tracy/Tracy.hpp>

pvp::SamplerBuilder& pvp::SamplerBuilder::set_filter(const VkFilter filter)
{
    m_sampler_info.magFilter = filter;
    m_sampler_info.minFilter = filter;
    return *this;
}

pvp::SamplerBuilder& pvp::SamplerBuilder::set_mipmap(const VkSamplerMipmapMode mode)
{
    m_sampler_info.mipmapMode = mode;
    return *this;
}

pvp::SamplerBuilder& pvp::SamplerBuilder::set_address_mode(const VkSamplerAddressMode mode)
{
    m_sampler_info.addressModeU = mode;
    m_sampler_info.addressModeV = mode;
    m_sampler_info.addressModeW = mode;
    return *this;
}

void pvp::SamplerBuilder::build(const Context& context, Sampler& sampler) const
{
    ZoneScoped;
    if (vkCreateSampler(context.device->get_device(), &m_sampler_info, nullptr, &sampler.handle) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture sampler!");
    }
}
