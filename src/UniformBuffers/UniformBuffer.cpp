#include "UniformBuffer.h"

namespace pvp
{
    UniformBuffer::UniformBuffer(size_t size, const VmaAllocator& allocator)
    {
        m_buffers.resize(MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            BufferBuilder()
                .set_size(size)
                .set_usage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
                .set_flags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT)
                .set_memory_usage(VMA_MEMORY_USAGE_AUTO)
                .build(allocator, m_buffers[i]);
        }
    }

    UniformBuffer::~UniformBuffer()
    {
        for (auto& buffer : m_buffers)
        {
            buffer.destroy();
        }
    }
} // namespace pvp