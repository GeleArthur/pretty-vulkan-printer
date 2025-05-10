#include "DescriptorLayout.h"

#include <vector>

namespace pvp
{
    const vk::DescriptorSetLayout& DescriptorLayout::get_handle() const
    {
        return m_layout;
    }
    void DescriptorLayout::destroy(const vk::Device& device) const
    {
        device.destroyDescriptorSetLayout(m_layout);
    }
} // namespace pvp
