#pragma once
#include <PVPPhysicalDevice/PVPPhysicalDevice.h>

#include "PVPInstance/PVPInstance.h"

class App
{
  public:
    void run();

  private:
    PVPInstance* m_pvp_instance{};
    PVPPhysicalDevice* m_pvp_physical_device{};
    DestructorQueue m_destructor_queue;
};
