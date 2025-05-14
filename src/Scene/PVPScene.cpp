#include "PVPScene.h"
#include "LoadModel.h"

#include <DestructorQueue.h>
#include <array>
#include <Buffer/BufferBuilder.h>
#include <CommandBuffer/CommandPool.h>
#include <GLFW/glfw3.h>
#include <GraphicsPipeline/Vertex.h>
#include <Renderer/Swapchain.h>
#include <VMAAllocator/VmaAllocator.h>
#include <Window/WindowSurface.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/mat4x4.hpp>
#include <spdlog/spdlog.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
pvp::PvpScene::PvpScene(Context& context)
    : context{ context }
{
    // auto models_loaded = load_model_file(std::filesystem::absolute("resources/Sponza/Sponza.gltf"));
    auto models_loaded = load_model_file(std::filesystem::absolute("resources/cube.obj"));

    const CommandPool     cmd_pool_transfer_buffers = CommandPool(context, *context.queue_families->get_queue_family(VK_QUEUE_TRANSFER_BIT, false), VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
    const VkCommandBuffer cmd = cmd_pool_transfer_buffers.begin_buffer();

    DestructorQueue transfer_buffer_deleter{};

    models.resize(models_loaded.size());
    for (int i = 0; i < models_loaded.size(); ++i)
    {
        LoadModel& model = models_loaded[i];
        Model&     new_model = models[i];

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

    scene_globals_gpu = new UniformBuffer<SceneGlobals>(context.allocator->get_allocator());

    // float               time = 0;
    // ModelCameraViewData ubo{};
    auto model = glm::rotate(glm::mat4(1.0f), 0.0f * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    camera_view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    camera_projection = glm::perspective(glm::radians(45.0f), static_cast<float>(context.swapchain->get_swapchain_extent().width) / static_cast<float>(context.swapchain->get_swapchain_extent().height), 0.1f, 10.0f);
    camera_projection[1][1] *= -1;

    SceneGlobals scene_globals{
        camera_projection * camera_view * model,
        glm::vec3{ 0, 0, 0 }
    };

    scene_globals_gpu->update(0, scene_globals);
}
pvp::PvpScene::~PvpScene()
{
    for (Model& model : models)
    {
        model.vertex_data.destroy();
        model.index_data.destroy();
    }

    delete scene_globals_gpu;
}
void pvp::PvpScene::update()
{
    glm::vec3 camera_position = glm::vec3{ 0, 0, 0 };

    if (glfwGetKey(context.window_surface->get_window(), GLFW_KEY_W) == GLFW_PRESS)
    {
        camera_position.z += 1.0f;
    }
    if (glfwGetKey(context.window_surface->get_window(), GLFW_KEY_S) == GLFW_PRESS)
    {
        camera_position.z += -1.0f;
    }
    if (glfwGetKey(context.window_surface->get_window(), GLFW_KEY_A) == GLFW_PRESS)
    {
        camera_position.x += -1.0f;
    }
    if (glfwGetKey(context.window_surface->get_window(), GLFW_KEY_D) == GLFW_PRESS)
    {
        camera_position.x += 1.0f;
    }

    // glm::vec3 new_location = scene_globals.camera_view_projection * glm::vec4(camera_position, 0.0);
    // scene_globals.camera_view_projection[0][3] += new_location.x;
    // scene_globals.camera_view_projection[1][3] += new_location.y;
    // scene_globals.camera_view_projection[2][3] += new_location.z;
    //
    // spdlog::info("{} {} {} {}", scene_globals.camera_view_projection[0][0], scene_globals.camera_view_projection[0][1], scene_globals.camera_view_projection[0][2], scene_globals.camera_view_projection[0][3]);
    //
    // scene_globals_gpu->update(0, scene_globals);
}
