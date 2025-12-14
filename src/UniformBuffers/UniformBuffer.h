#pragma once
#include <globalconst.h>
#include <vector>
#include <Buffer/Buffer.h>
#include <Buffer/BufferBuilder.h>

namespace pvp
{
    class UniformBuffer
    {
    public:
        explicit UniformBuffer() = default;
        explicit UniformBuffer(size_t size, const VmaAllocator& allocator);
        ~UniformBuffer();

        DISABLE_COPY(UniformBuffer);
        DISABLE_MOVE(UniformBuffer);

        template<typename T>
        void update(int frame_index, const T& data);

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
    void UniformBuffer::update(int frame_index, const T& data)
    {
        // TODO: Remove frame_index user should not worry about it
        m_buffers[frame_index].copy_data_into_buffer(std::span(&data, 1));
    }
} // namespace pvp
