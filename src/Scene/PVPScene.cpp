﻿#include "PVPScene.h"
#include "ModelData.h"

#include <DestructorQueue.h>
#include <fstream>
#include <iostream>
#include <set>
#include <stb_image.h>
#include <Buffer/BufferBuilder.h>
#include <CommandBuffer/CommandPool.h>
#include <Context/Device.h>
#include <DescriptorSets/DescriptorLayoutCreator.h>
#include <DescriptorSets/DescriptorLayoutBuilder.h>
#include <DescriptorSets/DescriptorSetBuilder.h>
#include <GLFW/glfw3.h>
#include <GraphicsPipeline/Vertex.h>
#include <Image/ImageBuilder.h>
#include <Image/SamplerBuilder.h>
#include <Renderer/Swapchain.h>
#include <VMAAllocator/VmaAllocator.h>
#include <assimp/material.h>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <spdlog/spdlog.h>
#include <tracy/Tracy.hpp>
pvp::PvpScene::PvpScene(Context& context)
    : m_context{ context }
    , m_camera(context)
{
    ZoneScoped;
    LoadedScene loaded_scene = load_scene_cpu(std::filesystem::absolute("resources/Sponza/Sponza.gltf"));
    // auto models_loaded = load_model_file(std::filesystem::absolute("resources/cube.obj"));

    const CommandPool     cmd_pool_transfer_buffers = CommandPool(context, *context.queue_families->get_queue_family(VK_QUEUE_TRANSFER_BIT, false), VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
    const VkCommandBuffer cmd = cmd_pool_transfer_buffers.begin_buffer();

    m_gpu_textures.reserve(loaded_scene.textures.size());
    std::vector<std::string> gpu_texture_names;
    gpu_texture_names.reserve(loaded_scene.textures.size());
    DestructorQueue transfer_image_deleter{};

    for (const TextureData& texture : loaded_scene.textures)
    {
        ZoneScopedN("Texture");
        ZoneTextF(texture.name.c_str());
        VkDeviceSize image_size = texture.width * texture.height * 4;

        Buffer staging_buffer{};
        BufferBuilder()
            .set_size(image_size)
            .set_usage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
            .set_flags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT)
            .build(context.allocator->get_allocator(), staging_buffer);

        transfer_image_deleter.add_to_queue([=] { staging_buffer.destroy(); });
        staging_buffer.copy_data_into_buffer(std::as_bytes(std::span(texture.pixels, image_size)));

        VkFormat format;
        switch (texture.type)
        {
            case aiTextureType_NORMALS:
                format = VK_FORMAT_R8G8B8A8_UNORM;
                break;
            default:
                format = VK_FORMAT_R8G8B8A8_SRGB;
        }

        StaticImage gpu_image;
        ImageBuilder()
            .set_name(texture.name)
            .set_format(format)
            .set_usage(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
            .set_size({ texture.width, texture.height })
            .set_memory_usage(VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE)
            .set_aspect_flags(VK_IMAGE_ASPECT_COLOR_BIT)
            .build(m_context, gpu_image);

        gpu_image.transition_layout(cmd,
                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                    VK_PIPELINE_STAGE_2_NONE,
                                    VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                                    VK_ACCESS_2_NONE,
                                    VK_ACCESS_2_TRANSFER_WRITE_BIT);
        gpu_image.copy_from_buffer(cmd, staging_buffer);
        gpu_image.transition_layout(cmd,
                                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                    VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                                    VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                                    VK_ACCESS_2_TRANSFER_WRITE_BIT,
                                    VK_ACCESS_2_SHADER_READ_BIT);

        stbi_image_free(texture.pixels);

        m_gpu_textures.push_back(std::move(gpu_image));
        gpu_texture_names.push_back(texture.name);
    }

    DestructorQueue transfer_buffer_deleter{};
    m_gpu_models.resize(loaded_scene.models.size());
    for (int i = 0; i < loaded_scene.models.size(); ++i)
    {
        ZoneScopedN("Model");
        ModelData& cpu_model = loaded_scene.models[i];
        Model&     gpu_model = m_gpu_models[i];

        gpu_model.index_count = cpu_model.indices.size();

        // Vertex loading
        Buffer transfer_buffer{};
        BufferBuilder()
            .set_size(cpu_model.vertices.size() * sizeof(Vertex))
            .set_usage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
            .set_flags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT)
            .set_memory_usage(VMA_MEMORY_USAGE_AUTO_PREFER_HOST)
            .build(context.allocator->get_allocator(), transfer_buffer);

        transfer_buffer.copy_data_into_buffer(std::as_bytes(std::span(cpu_model.vertices)));
        transfer_buffer_deleter.add_to_queue([=] { transfer_buffer.destroy(); });

        BufferBuilder()
            .set_size(cpu_model.vertices.size() * sizeof(Vertex))
            .set_usage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
            .set_memory_usage(VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE)
            .build(context.allocator->get_allocator(), gpu_model.vertex_data);

        gpu_model.vertex_data.copy_from_buffer(cmd, transfer_buffer);

        // Index loading
        Buffer transfer_buffer_index{};
        BufferBuilder()
            .set_size(cpu_model.vertices.size() * sizeof(Vertex))
            .set_usage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
            .set_flags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT)
            .build(context.allocator->get_allocator(), transfer_buffer_index);

        transfer_buffer_index.copy_data_into_buffer(std::as_bytes(std::span(cpu_model.indices)));
        transfer_buffer_deleter.add_to_queue([=] { transfer_buffer_index.destroy(); });

        BufferBuilder()
            .set_size(cpu_model.indices.size() * sizeof(uint32_t))
            .set_usage(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
            .build(context.allocator->get_allocator(), gpu_model.index_data);

        gpu_model.index_data.copy_from_buffer(cmd, transfer_buffer_index);

        gpu_model.material.transform = cpu_model.transform;

        // :(
        if (cpu_model.diffuse_path.empty())
            gpu_model.material.diffuse_texture_index = 0;
        else
            gpu_model.material.diffuse_texture_index = std::ranges::find(gpu_texture_names, cpu_model.diffuse_path) - gpu_texture_names.begin();

        if (cpu_model.normal_path.empty())
            gpu_model.material.normal_texture_index = 0;
        else
            gpu_model.material.normal_texture_index = std::ranges::find(gpu_texture_names, cpu_model.normal_path) - gpu_texture_names.begin();

        if (cpu_model.metallic_path.empty())
            gpu_model.material.metalness_texture_index = 0;
        else
            gpu_model.material.metalness_texture_index = std::ranges::find(gpu_texture_names, cpu_model.metallic_path) - gpu_texture_names.begin();
    }
    cmd_pool_transfer_buffers.end_buffer(cmd);
    transfer_buffer_deleter.destroy_and_clear();
    transfer_image_deleter.destroy_and_clear();
    cmd_pool_transfer_buffers.destroy();

    m_scene_globals_gpu = new UniformBuffer<SceneGlobals>(context.allocator->get_allocator());

    context.descriptor_creator->create_layout()
        .add_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
        .build(0);

    context.descriptor_creator->create_layout()
        .add_flag(0)
        .add_binding(VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .add_flag(VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT)
        .add_binding(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT, 1000)
        .build(1);

    SamplerBuilder()
        .set_filter(VK_FILTER_LINEAR)
        .set_mipmap(VK_SAMPLER_MIPMAP_MODE_LINEAR)
        .set_address_mode(VK_SAMPLER_ADDRESS_MODE_REPEAT)
        .build(m_context, m_shadered_sampler);

    m_scene_binding = DescriptorSetBuilder()
                          .set_layout(m_context.descriptor_creator->get_layout(0))
                          .bind_buffer(0, *m_scene_globals_gpu)
                          .build(m_context);

    m_all_textures = DescriptorSetBuilder()
                         .set_layout(m_context.descriptor_creator->get_layout(1))
                         .bind_sampler(0, m_shadered_sampler)
                         .bind_image_array(1, m_gpu_textures)
                         .build(context);

    BufferBuilder()
        .set_usage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
        .set_flags(VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT)
        .set_memory_usage(VMA_MEMORY_USAGE_AUTO)
        .set_size((16 + sizeof(PointLight) * max_point_lights))
        .build(m_context.allocator->get_allocator(), m_point_lights);

    BufferBuilder()
        .set_usage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
        .set_flags(VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT)
        .set_memory_usage(VMA_MEMORY_USAGE_AUTO)
        .set_size(16 + sizeof(DirectionLight) * max_direction_lights)
        .build(m_context.allocator->get_allocator(), m_directonal_lights);

    m_context.descriptor_creator->create_layout()
        .add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .build(3);

    std::vector buffers{ m_point_lights };
    std::vector buffer_direction{ m_directonal_lights };

    m_point_descriptor = DescriptorSetBuilder()
                             .set_layout(m_context.descriptor_creator->get_layout(3))
                             .bind_buffer_ssbo(0, buffers)
                             .bind_buffer_ssbo(1, buffer_direction)
                             .build(m_context);

    direction_light = DirectionLight{ { 0.557f, -0.557f, -0.557f, 0 }, { 1, 1, 1, 1.0f }, 0 };

    add_point_light(PointLight{ { 3, 0, 0, 0 }, { 1, 0, 0, 1.0f }, 100 });
    add_point_light(PointLight{ { 10, 2, -0.25f, 0 }, { 0, 1, 0, 1.0f }, 500 });
    add_direction_light(direction_light);
}
pvp::PvpScene::~PvpScene()
{
    for (Model& model : m_gpu_models)
    {
        model.vertex_data.destroy();
        model.index_data.destroy();
    }

    for (StaticImage& gpu_texture : m_gpu_textures)
    {
        gpu_texture.destroy(m_context);
    }

    delete m_scene_globals_gpu;

    m_shadered_sampler.destroy(m_context.device->get_device());
    m_point_lights.destroy();
    m_directonal_lights.destroy();
}
uint32_t pvp::PvpScene::add_point_light(const PointLight& light) const
{
    void*     light_base = m_point_lights.get_allocation_info().pMappedData;
    uint32_t  light_index = (++*static_cast<uint32_t*>(light_base));
    std::span point_lights(reinterpret_cast<PointLight*>(reinterpret_cast<char*>(light_base) + 16u), max_point_lights);
    point_lights[light_index - 1] = light;

    return light_index - 1;
}
void pvp::PvpScene::change_point_light(uint32_t index, const PointLight& light) const
{
    void* light_base = m_point_lights.get_allocation_info().pMappedData;

    std::span point_lights(reinterpret_cast<PointLight*>(reinterpret_cast<char*>(light_base) + 16u), max_direction_lights);
    point_lights[index] = light;
}

uint32_t pvp::PvpScene::add_direction_light(const DirectionLight& light) const
{
    void* light_base = m_directonal_lights.get_allocation_info().pMappedData;

    uint32_t  light_index = (++*static_cast<uint32_t*>(light_base));
    std::span point_lights(reinterpret_cast<DirectionLight*>(reinterpret_cast<char*>(light_base) + 16u), max_direction_lights);
    point_lights[light_index - 1] = light;

    return light_index - 1;
}
void pvp::PvpScene::change_direction_light(uint32_t index, const DirectionLight& light) const
{
    void* light_base = m_directonal_lights.get_allocation_info().pMappedData;

    std::span point_lights(reinterpret_cast<DirectionLight*>(reinterpret_cast<char*>(light_base) + 16u), max_direction_lights);
    point_lights[index] = light;
}

void pvp::PvpScene::update()
{
}
void pvp::PvpScene::update_render()
{
    static auto start_time = std::chrono::high_resolution_clock::now();
    static auto last_time = std::chrono::high_resolution_clock::now();
    const auto  current_time = std::chrono::high_resolution_clock::now();
    const float delta_time = std::chrono::duration<float>(current_time - last_time).count();
    const float time = std::chrono::duration<float>(current_time - start_time).count();
    last_time = current_time;

    m_camera.update(delta_time);

    m_scene_globals = {
        m_camera.get_view_matrix(),
        m_camera.get_projection_matrix(),
        m_camera.get_position()
    };

    PointLight light = { glm::vec4(std::sin(time), 1.0f, std::cos(time), 0.0), { 1.0f, 0.5f, 0, 1 }, 150 };
    change_point_light(0, light);

    static bool key_pressed_last{};
    bool        key_pressed = glfwGetKey(m_context.window_surface->get_window(), GLFW_KEY_SPACE) == GLFW_PRESS;

    if (key_pressed && !key_pressed_last)
    {
        direction_light.intensity = direction_light.intensity > 5.0f ? 0.0f : 10.0f;
        change_direction_light(0, direction_light);
    }
    key_pressed_last = key_pressed;
    m_scene_globals_gpu->update(0, m_scene_globals);
}
