#pragma once

#include <vma/vk_mem_alloc.h>

namespace pvp
{
    class PhysicalDevice;
    class Device;
    class Instance;

    class PvpVmaAllocator
    {
    public:
        [[nodiscard]] const VmaAllocator& get_allocator() const;
        ~PvpVmaAllocator();

    private:
        friend void  create_allocator(PvpVmaAllocator& allocator, const pvp::Instance& instance, const pvp::Device& device, const pvp::PhysicalDevice& physical_device);
        VmaAllocator m_allocator{ VK_NULL_HANDLE };
    };

    void create_allocator(PvpVmaAllocator& allocator, const pvp::Instance& instance, const pvp::Device& device, const pvp::PhysicalDevice& physical_device);
} // namespace pvp
