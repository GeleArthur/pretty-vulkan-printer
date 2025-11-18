#include "VulkanExternalFunctions.h"
void VulkanInstanceExtensions::register_instance(const VkInstance instance)
{
    my_instance = instance;
}
void VulkanInstanceExtensions::register_device(VkDevice device)
{
    my_device = device;
}