#ifndef APP_H
#define APP_H
#include "Instance/VulkanInstance.h"

class App
{
  public:
    void run();

  private:
    InstanceContext m_instance_context{};
    DestructorQueue m_destructor_queue;
};

#endif // APP_H
