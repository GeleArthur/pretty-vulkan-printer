#pragma once
#include <DestructorQueue.h>
#include <vulkan/vulkan.h>

namespace pvp
{
    class Instance
    {
    public:
        explicit Instance() = default;
        const VkInstance& get_instance() const;
        void              destroy();

    private:
        friend class InstanceBuilder;
        VkInstance      m_instance{ nullptr };
        DestructorQueue m_destructor_queue{};
    };
} // namespace pvp
