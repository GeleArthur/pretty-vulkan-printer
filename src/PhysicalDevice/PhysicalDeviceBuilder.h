#pragma once
#include "PhysicalDevice.h"
namespace pvp
{
    class Instance;
    class PhysicalDeviceBuilder final
    {
    public:
        explicit PhysicalDeviceBuilder() = default;

        void build(const Instance& instance, PhysicalDevice& physical_device);

    private:
    };
} // namespace pvp
