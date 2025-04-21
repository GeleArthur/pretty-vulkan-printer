#pragma once
#include <Context.h>

class PVPInstanceBuilder
{
  public:
    void Build(pvp::Context& context);

  private:
    std::string m_app_name{"pretty vulkan printer"};
    int width{800};
    int height{600};
};
