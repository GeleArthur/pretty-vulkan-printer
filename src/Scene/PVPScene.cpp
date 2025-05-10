#include "PVPScene.h"
#include "LoadModel.h"

#include <array>
#include <PVPBuffer/BufferBuilder.h>
#include <PVPCommandBuffer/CommandPool.h>
#include <PVPGraphicsPipeline/Vertex.h>
pvp::PvpScene pvp::load_scene(const Context& context)
{
    PvpScene scene{};

    std::array models = {
        load_model_file(std::filesystem::absolute("resources/viking_room.obj"))
    };
    scene.models.resize(models.size());

    const CommandPool     cmd_pool_transfer_buffers = CommandPool(context, *context.queue_families->get_queue_family(VK_QUEUE_TRANSFER_BIT, false), VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
    const VkCommandBuffer cmd = cmd_pool_transfer_buffers.begin_buffer();

    DestructorQueue transfer_buffer_deleter{};

    for (int i = 0; i < models.size(); ++i)
    {
        LoadModel& model = models[i];
        Model&     new_model = scene.models[i];

        new_model.index_count = model.indices.size();

        // Vertex loading
        Buffer transfer_buffer{};
        BufferBuilder()
            .set_size(model.verties.size() * sizeof(Vertex))
            .set_usage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
            .set_flags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT)
            .build(context.allocator->get_allocator(), transfer_buffer);

        transfer_buffer.copy_data_into_buffer(std::as_bytes(std::span(model.verties)));
        transfer_buffer_deleter.add_to_queue([&] { transfer_buffer.destroy(); });

        BufferBuilder()
            .set_size(model.verties.size() * sizeof(Vertex))
            .set_usage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
            .build(context.allocator->get_allocator(), new_model.vertex_data);

        new_model.vertex_data.copy_from_buffer(cmd, transfer_buffer);

        // Index loading
        Buffer transfer_buffer_index{};
        BufferBuilder()
            .set_size(model.verties.size() * sizeof(Vertex))
            .set_usage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
            .set_flags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT)
            .build(context.allocator->get_allocator(), transfer_buffer_index);

        transfer_buffer_index.copy_data_into_buffer(std::as_bytes(std::span(model.indices)));
        transfer_buffer_deleter.add_to_queue([&] { transfer_buffer_index.destroy(); });

        BufferBuilder()
            .set_size(model.indices.size() * sizeof(uint32_t))
            .set_usage(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
            .build(context.allocator->get_allocator(), new_model.index_data);

        new_model.index_data.copy_from_buffer(cmd, transfer_buffer_index);
    }
    cmd_pool_transfer_buffers.end_buffer(cmd);
    transfer_buffer_deleter.destroy_and_clear();
    cmd_pool_transfer_buffers.destroy();

    return scene;
}
void pvp::destroy_scene(PvpScene& scene)
{
    for (Model& model : scene.models)
    {
        model.vertex_data.destroy();
        model.index_data.destroy();
    }
}