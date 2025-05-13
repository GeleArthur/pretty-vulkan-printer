#pragma once
#include <globalconst.h>
#include <vector>
#include <Buffer/Buffer.h>
#include <Buffer/BufferBuilder.h>

namespace pvp
{
    template<typename T>
    class UniformBuffer
    {
    public:
        explicit UniformBuffer(const VmaAllocator& allocator);
        ~UniformBuffer();

        void update(uint32_t frame_index, const T& data);

        [[nodiscard]] const Buffer& get_buffer(uint32_t frame_index) const
        {
            return m_buffers[frame_index];
        }
        [[nodiscard]] const std::vector<Buffer>& get_buffers() const
        {
            return m_buffers;
        }

    private:
        std::vector<Buffer> m_buffers;
    };

    template<typename T>
    UniformBuffer<T>::UniformBuffer(const VmaAllocator& allocator)
    {
        m_buffers.resize(MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            BufferBuilder()
                .set_size(sizeof(T))
                .set_usage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
                .set_flags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT)
                .build(allocator, m_buffers[i]);
        }
    }

    template<typename T>
    UniformBuffer<T>::~UniformBuffer()
    {
        for (auto& buffer : m_buffers)
        {
            buffer.destroy();
        }
    }

    template<typename T>
    void UniformBuffer<T>::update(uint32_t frame_index, const T& data)
    {
        // TODO: Remove frame_index user should not worry about it
        m_buffers[frame_index].copy_data_into_buffer(std::as_bytes(std::span(&data, 1)));
    }
} // namespace pvp
