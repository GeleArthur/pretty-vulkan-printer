#pragma once
#include <vulkan/vulkan.h>

namespace pvp
{
    struct DescriptorSets;
    class Buffer;
    class GpuModel
    {
        public:
        explicit GpuModel(Buffer vertex_buffer, Buffer index_buffer);
        void draw(VkCommandBuffer command_buffer, VkPipelineLayout pipeline);

        private:
        Buffer*        m_vertex_buffer;
        Buffer*        m_index_buffer;
        DescriptorSets* m_uniform_descriptor;
        uint32_t       m_index_count;
    };
} // namespace pvp
