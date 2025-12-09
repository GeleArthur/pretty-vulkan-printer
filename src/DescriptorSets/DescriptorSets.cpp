#include "DescriptorSets.h"

#include "DescriptorLayoutCreator.h"

#include <Context/Device.h>

void pvp::DescriptorSets::destroy() const
{
    vkFreeDescriptorSets(m_context->device->get_device(),
                         m_context->descriptor_creator->get_pool(),
                         m_sets.size(),
                         m_sets.data());
}

void pvp::DescriptorSets::reconnect_image(const ImageBinding& binding)
{
    VkDescriptorImageInfo image_info{};
    image_info.imageView = binding.image->get_view(binding.set);
    image_info.imageLayout = binding.layout;

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = m_sets[binding.set];
    write.dstBinding = binding.binding;
    write.dstArrayElement = 0;
    write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    write.descriptorCount = 1;
    write.pImageInfo = &image_info;

    vkUpdateDescriptorSets(m_context->device->get_device(), 1, &write, 0, nullptr);
}