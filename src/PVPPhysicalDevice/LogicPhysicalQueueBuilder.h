#pragma once
#include <vector>

#include "PhysicalDevice.h"
#include "QueueFamillies.h"

#include <complex.h>

namespace pvp
{
    class Device;
    class PhysicalDevice;
    class WindowSurface;
    class Instance;

    class LogicPhysicalQueueBuilder final
    {
    public:
        explicit LogicPhysicalQueueBuilder() = default;

        void build(const Instance& instance, const WindowSurface& window_surface, PhysicalDevice& physical_device, Device& device, QueueFamilies& queue_families);

    private:
        [[nodiscard]] VkPhysicalDevice                get_best_device(const Instance& instance, const WindowSurface& window_surface) const;
        std::tuple<bool, QueueFamilies> get_queue_family_indices(const VkPhysicalDevice& physical_device);
        std::vector<const char*>        m_extentions;
    };
} // namespace pvp
