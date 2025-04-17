#include "App.h"

#include <iostream>

void App::run()
{

    // TODO: Debug stuff and swapchain extensions should be moved inside as it a given that we want it.
    m_pvp_instance = new PVPInstance(
        800,
        800,
        "pretty vulkan printer",
        true,
        {VK_EXT_DEBUG_UTILS_EXTENSION_NAME},
        {"VK_LAYER_KHRONOS_validation"});
    m_destructor_queue.add_to_queue([&] { delete m_pvp_instance; });

    m_pvp_physical_device = new PVPPhysicalDevice(m_pvp_instance, {});
    m_destructor_queue.add_to_queue([&] { delete m_pvp_physical_device; });
}
