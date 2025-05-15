#include "PVPScene.h"
#include "LoadModel.h"

#include <DestructorQueue.h>
#include <Buffer/BufferBuilder.h>
#include <CommandBuffer/CommandPool.h>
#include <GLFW/glfw3.h>
#include <GraphicsPipeline/Vertex.h>
#include <Renderer/Swapchain.h>
#include <VMAAllocator/VmaAllocator.h>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <spdlog/spdlog.h>
pvp::PvpScene::PvpScene(Context& context)
    : m_context{ context }
    , m_camera(context)
{
    auto models_loaded = load_model_file(std::filesystem::absolute("resources/Sponza/Sponza.gltf"));
    // auto models_loaded = load_model_file(std::filesystem::absolute("resources/cube.obj"));

    const CommandPool     cmd_pool_transfer_buffers = CommandPool(context, *context.queue_families->get_queue_family(VK_QUEUE_TRANSFER_BIT, false), VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
    const VkCommandBuffer cmd = cmd_pool_transfer_buffers.begin_buffer();

    DestructorQueue transfer_buffer_deleter{};

    m_models.resize(models_loaded.size());
    for (int i = 0; i < models_loaded.size(); ++i)
    {
        LoadModel& model = models_loaded[i];
        Model&     new_model = m_models[i];

        new_model.transform = model.transform;
        new_model.index_count = model.indices.size();

        // Vertex loading
        Buffer transfer_buffer{};
        BufferBuilder()
            .set_size(model.vertices.size() * sizeof(Vertex))
            .set_usage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
            .set_flags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT)
            .build(context.allocator->get_allocator(), transfer_buffer);

        transfer_buffer.copy_data_into_buffer(std::as_bytes(std::span(model.vertices)));
        transfer_buffer_deleter.add_to_queue([=] { transfer_buffer.destroy(); });

        BufferBuilder()
            .set_size(model.vertices.size() * sizeof(Vertex))
            .set_usage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
            .build(context.allocator->get_allocator(), new_model.vertex_data);

        new_model.vertex_data.copy_from_buffer(cmd, transfer_buffer);

        // Index loading
        Buffer transfer_buffer_index{};
        BufferBuilder()
            .set_size(model.vertices.size() * sizeof(Vertex))
            .set_usage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
            .set_flags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT)
            .build(context.allocator->get_allocator(), transfer_buffer_index);

        transfer_buffer_index.copy_data_into_buffer(std::as_bytes(std::span(model.indices)));
        transfer_buffer_deleter.add_to_queue([=] { transfer_buffer_index.destroy(); });

        BufferBuilder()
            .set_size(model.indices.size() * sizeof(uint32_t))
            .set_usage(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
            .build(context.allocator->get_allocator(), new_model.index_data);

        new_model.index_data.copy_from_buffer(cmd, transfer_buffer_index);
    }
    cmd_pool_transfer_buffers.end_buffer(cmd);
    transfer_buffer_deleter.destroy_and_clear();
    cmd_pool_transfer_buffers.destroy();

    m_scene_globals_gpu = new UniformBuffer<SceneGlobals>(context.allocator->get_allocator());
}
pvp::PvpScene::~PvpScene()
{
    for (Model& model : m_models)
    {
        model.vertex_data.destroy();
        model.index_data.destroy();
    }

    delete m_scene_globals_gpu;
}
void pvp::PvpScene::update()
{
    static auto start_time = std::chrono::high_resolution_clock::now();
    static auto last_time = std::chrono::high_resolution_clock::now();
    const auto  current_time = std::chrono::high_resolution_clock::now();
    const float delta_time = std::chrono::duration<float>(current_time - last_time).count();
    const float time = std::chrono::duration<float>(current_time - start_time).count();

    last_time = current_time;

    m_camera.update(delta_time);

    glm::mat4x4 model = glm::rotate(glm::mat4(1.0f), 0.0f * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    SceneGlobals scene_globals{
        m_camera.get_perspective_matrix() * m_camera.get_view_matrix() * model,
        glm::vec3{ 0, 0, 0 }
    };

    m_scene_globals_gpu->update(0, scene_globals);
}
