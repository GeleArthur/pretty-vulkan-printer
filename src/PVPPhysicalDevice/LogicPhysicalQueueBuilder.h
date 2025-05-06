#pragma once
#include <vector>

#include "PhysicalDevice.h"
#include "QueueFamilies.h"

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
        LogicPhysicalQueueBuilder& set_extensions(const std::vector<const char*>& extension);

        void build(const Instance& instance, const WindowSurface& window_surface, PhysicalDevice& physical_device_out, Device& device_out, QueueFamilies& queue_families_out);

    private:
        [[nodiscard]] VkPhysicalDevice get_best_device(const Instance& instance, const WindowSurface& window_surface) const;
        [[nodiscard]] bool             is_supports_all_queues(const VkPhysicalDevice& physical_device, const WindowSurface& window_surface) const;
        std::vector<const char*>       m_extensions;
    };
} // namespace pvp
