#pragma once
#include <string>
#include <vector>

#include "Instance.h"

namespace pvp
{
    class InstanceBuilder
    {
    public:
        InstanceBuilder& set_app_name(const std::string& name);
        InstanceBuilder& enable_validation_layers(bool enabled);

        void build(Instance& context);

    private:
        void valid_extensions_check();

        std::string m_app_name{ "pretty vulkan printer" };

        bool                     m_enable_validation_layers{ false };
        std::vector<const char*> m_extensions;
        std::vector<const char*> m_layers;
    };
} // namespace pvp
