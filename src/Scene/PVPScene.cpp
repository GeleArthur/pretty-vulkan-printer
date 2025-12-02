#include "PVPScene.h"
#include "ModelData.h"

#include <DestructorQueue.h>
#include <VulkanExternalFunctions.h>
#include <imgui.h>
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
#include <glm/gtx/rotate_vector.hpp>
#include <spdlog/spdlog.h>
#include <tracy/Tracy.hpp>
#include <glm/glm.hpp>

static void generate_default_textures()
{
}

pvp::PvpScene::PvpScene(Context& context)
    : m_context{ context }
    , m_scene_globals{}
    , m_point_lights_gpu{ 16 + sizeof(PointLight) * max_point_lights, context.allocator->get_allocator() }
    , m_directonal_lights_gpu{ 16 + sizeof(DirectionLight) * max_direction_lights, context.allocator->get_allocator() }
    , m_scene_globals_gpu{ sizeof(SceneGlobals), context.allocator->get_allocator() }
    , m_camera(context)
{
    ZoneScoped;

    m_command_queue.resize(max_frames_in_flight);

    LoadedScene loaded_scene = load_scene_cpu(std::filesystem::absolute("resources/Sponza/Sponza.gltf"));
    // LoadedScene loaded_scene = load_scene_cpu(std::filesystem::absolute("resources/rossbandiger/Fixed mesh.glb"));
    // LoadedScene loaded_scene = load_scene_cpu(std::filesystem::absolute("resources/test_triangle.glb"));
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
        VkDeviceSize image_size = texture.width * texture.height * texture.channels;

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
            case aiTextureType_DIFFUSE:
                format = VK_FORMAT_R8G8B8A8_SRGB;
                break;
            default:
                format = VK_FORMAT_R8G8B8A8_UNORM;
        }

        StaticImage gpu_image;
        ImageBuilder()
            .set_name(texture.name)
            .set_format(format)
            .set_usage(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
            .set_size({ texture.width, texture.height })
            .set_memory_usage(VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE)
            .set_aspect_flags(VK_IMAGE_ASPECT_COLOR_BIT)
            .set_use_mipmap(true)
            .build(m_context, gpu_image);

        gpu_image.transition_layout(cmd,
                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                    VK_PIPELINE_STAGE_2_NONE,
                                    VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                                    VK_ACCESS_2_NONE,
                                    VK_ACCESS_2_TRANSFER_WRITE_BIT);
        gpu_image.copy_from_buffer(cmd, staging_buffer);
        gpu_image.transition_layout(cmd,
                                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                    VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                                    VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                                    VK_ACCESS_2_TRANSFER_WRITE_BIT,
                                    VK_ACCESS_2_TRANSFER_READ_BIT);

        stbi_image_free(texture.pixels);

        for (uint32_t i = 1; i < gpu_image.get_mipmap_levels(); ++i)
        {
            VkImageBlit2 region{
                .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
                .srcSubresource = VkImageSubresourceLayers{
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = i - 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1 },
                .srcOffsets = { VkOffset3D{ 0, 0, 0 }, VkOffset3D{ static_cast<int32_t>(texture.width >> (i - 1)), static_cast<int32_t>(texture.height >> (i - 1)), 1 } },
                .dstSubresource = VkImageSubresourceLayers{ .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = i, .baseArrayLayer = 0, .layerCount = 1 },
                .dstOffsets = { VkOffset3D{ 0, 0, 0 }, VkOffset3D{ static_cast<int32_t>(texture.width >> i), static_cast<int32_t>(texture.height >> i), 1 } },
            };

            const VkBlitImageInfo2 info{
                .sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
                .srcImage = gpu_image.get_image(),
                .srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                .dstImage = gpu_image.get_image(),
                .dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .regionCount = 1,
                .pRegions = &region,
                .filter = VK_FILTER_NEAREST
            };

            VkImageSubresourceRange range{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = i,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            };

            gpu_image.transition_layout_range(cmd,
                                              VK_IMAGE_LAYOUT_UNDEFINED,
                                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                              VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
                                              VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                                              VK_ACCESS_2_NONE,
                                              VK_ACCESS_2_TRANSFER_WRITE_BIT,
                                              range);
            vkCmdBlitImage2(cmd, &info);

            gpu_image.transition_layout_range(cmd,
                                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                              VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                              VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                                              VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                                              VK_ACCESS_2_TRANSFER_WRITE_BIT,
                                              VK_ACCESS_2_TRANSFER_READ_BIT,
                                              range);
        }

        gpu_image.transition_layout(cmd,
                                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                    VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                                    VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                                    VK_ACCESS_2_TRANSFER_READ_BIT,
                                    VK_ACCESS_2_SHADER_READ_BIT);

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

        auto transfer_to_gpu = [&context, &transfer_buffer_deleter, &cmd]<typename T>(const std::span<T> data, Buffer& gpu_buffer, VkBufferUsageFlags usage, const std::string& name = "") {
            Buffer transfer_buffer{};
            BufferBuilder()
                .set_size(data.size_bytes())
                .set_usage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
                .set_flags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT)
                .set_memory_usage(VMA_MEMORY_USAGE_AUTO_PREFER_HOST)
                .build(context.allocator->get_allocator(), transfer_buffer);

            transfer_buffer.copy_data_into_buffer(std::as_bytes(data));
            transfer_buffer_deleter.add_to_queue([transfer_buffer] {
                transfer_buffer.destroy();
            });

            BufferBuilder()
                .set_size(data.size_bytes())
                .set_usage(usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
                .set_memory_usage(VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE)
                .build(context.allocator->get_allocator(), gpu_buffer);
            gpu_buffer.copy_from_buffer(cmd, transfer_buffer);

            if (!name.empty())
            {
                VkDebugMarkerObjectNameInfoEXT nameInfo{};
                nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
                nameInfo.pNext = nullptr;
                nameInfo.objectType = VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT;
                nameInfo.object = (uint64_t)(gpu_buffer.get_buffer());
                nameInfo.pObjectName = name.c_str();

                VulkanInstanceExtensions::vkDebugMarkerSetObjectNameEXT(context.device->get_device(), &nameInfo);
            }
        };

        transfer_to_gpu(std::span(cpu_model.vertices), gpu_model.vertex_data, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, "Mesh vertex data");

        // Index loading
        transfer_to_gpu(std::span(cpu_model.indices), gpu_model.index_data, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, "indcies");
        gpu_model.index_count = cpu_model.indices.size();
        gpu_model.meshlet_count = cpu_model.meshlets.size();

        // meshletes loading
        transfer_to_gpu(std::span<meshopt_Meshlet>(cpu_model.meshlets), gpu_model.meshlet_buffer, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, "Meshlets");
        // transfer_to_gpu(std::span<Vertex>(cpu_model.vertices), gpu_model.meshlet_vertex, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
        transfer_to_gpu(std::span(cpu_model.meshlet_triangles), gpu_model.meshlet_triangles_buffer, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, "Meshlet triangle");
        transfer_to_gpu(std::span(cpu_model.meshlet_vertices), gpu_model.meshlet_vertices_buffer, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, "Meshlet vertex index");
        transfer_to_gpu(std::span(cpu_model.meshlet_sphere_bounds), gpu_model.meshlet_sphere_bounds_buffer, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, "Meshlet sphere bounds");

        DescriptorSetBuilder{}
            .set_layout(m_context.descriptor_creator->get_layout()
                            .add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_TASK_BIT_EXT)
                            .add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_TASK_BIT_EXT)
                            .add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_TASK_BIT_EXT)
                            .add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_TASK_BIT_EXT)
                            .add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_TASK_BIT_EXT)
                            .set_tag(DiscriptorTag::meshlets)
                            .get())
            .bind_buffer_ssbo(0, gpu_model.meshlet_buffer)
            .bind_buffer_ssbo(1, gpu_model.vertex_data)
            .bind_buffer_ssbo(2, gpu_model.meshlet_vertices_buffer)
            .bind_buffer_ssbo(3, gpu_model.meshlet_triangles_buffer)
            .bind_buffer_ssbo(4, gpu_model.meshlet_sphere_bounds_buffer)
            .build(m_context, gpu_model.meshlet_descriptor_set);

        gpu_model.material.transform = cpu_model.transform;

        // TODO: Better default selection
        gpu_model.material.diffuse_texture_index = cpu_model.diffuse_path.empty() ? 0 : std::ranges::find(gpu_texture_names, cpu_model.diffuse_path) - gpu_texture_names.begin();
        gpu_model.material.normal_texture_index = cpu_model.normal_path.empty() ? 0 : std::ranges::find(gpu_texture_names, cpu_model.normal_path) - gpu_texture_names.begin();
        gpu_model.material.metalness_texture_index = cpu_model.metallic_path.empty() ? 0 : std::ranges::find(gpu_texture_names, cpu_model.metallic_path) - gpu_texture_names.begin();
    }
    cmd_pool_transfer_buffers.end_buffer(cmd);
    transfer_buffer_deleter.destroy_and_clear();
    transfer_image_deleter.destroy_and_clear();
    cmd_pool_transfer_buffers.destroy();

    // m_scene_globals_gpu = UniformBuffer{};
    // m_point_lights_gpu = UniformBuffer(16 + sizeof(PointLight) * max_point_lights, context.allocator->get_allocator());
    // m_directonal_lights_gpu = UniformBuffer(16 + sizeof(DirectionLight) * max_direction_lights, context.allocator->get_allocator());

    SamplerBuilder()
        .set_filter(VK_FILTER_LINEAR)
        .set_mipmap(VK_SAMPLER_MIPMAP_MODE_LINEAR)
        .set_address_mode(VK_SAMPLER_ADDRESS_MODE_REPEAT)
        .build(m_context, m_shadered_sampler);

    DescriptorSetBuilder()
        .set_layout(context.descriptor_creator->get_layout()
                        .add_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_TASK_BIT_EXT)
                        .set_tag(DiscriptorTag::scene_globals)
                        .get())
        .bind_uniform_buffer(0, m_scene_globals_gpu)
        .build(m_context, m_scene_binding);

    DescriptorSetBuilder()
        .set_layout(context.descriptor_creator->get_layout()
                        .add_flag(0)
                        .add_binding(VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_MESH_BIT_EXT)
                        .add_flag(VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT)
                        .add_binding(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_MESH_BIT_EXT, 140)
                        .set_tag(DiscriptorTag::bindless_textures)
                        .get())
        .bind_sampler(0, m_shadered_sampler)
        .bind_image_array(1, m_gpu_textures)
        .build(context, m_all_textures);

    DescriptorSetBuilder()
        .set_layout(m_context.descriptor_creator->get_layout()
                        .add_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
                        .add_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
                        .set_tag(DiscriptorTag::lights)
                        .get())
        .bind_uniform_buffer(0, m_point_lights_gpu)
        .bind_uniform_buffer(1, m_directonal_lights_gpu)
        .build(m_context, m_point_descriptor);

    m_direction_light = DirectionLight{ { 0.557f, -0.557f, -0.557f, 0 }, { 1, 1, 1, 1.0f }, 100 };

    add_point_light(PointLight{ { 3, 0, 0, 0 }, { 1, 0, 0, 1.0f }, 100 });
    add_point_light(PointLight{ { 10, 2, -0.25f, 0 }, { 0, 1, 0, 1.0f }, 500 });
    add_direction_light(m_direction_light);
}

pvp::PvpScene::~PvpScene()
{
    for (Model& model : m_gpu_models)
    {
        model.vertex_data.destroy();
        model.index_data.destroy();
        // model.meshlet_vertex.destroy();
        model.meshlet_buffer.destroy();
        model.meshlet_triangles_buffer.destroy();
        model.meshlet_vertices_buffer.destroy();
        model.meshlet_sphere_bounds_buffer.destroy();
    }

    for (StaticImage& gpu_texture : m_gpu_textures)
    {
        gpu_texture.destroy(m_context);
    }

    m_shadered_sampler.destroy(m_context.device->get_device());
}

uint32_t pvp::PvpScene::add_point_light(const PointLight& light)
{
    for (std::vector<std::function<void(int, PvpScene&)>>& command_queue : m_command_queue)
    {
        command_queue.push_back([light](int buffer_index, PvpScene& scene) {
            void*          light_base = scene.m_point_lights_gpu.get_buffer(buffer_index).get_allocation_info().pMappedData;
            const uint32_t light_index = (++*static_cast<uint32_t*>(light_base));
            std::span      point_lights(reinterpret_cast<PointLight*>(reinterpret_cast<char*>(light_base) + 16u), max_point_lights);
            point_lights[light_index - 1] = light;
        });
    }
    return 0;
}

void pvp::PvpScene::change_point_light(uint32_t light_index, const PointLight& light)
{
    for (std::vector<std::function<void(int, PvpScene&)>>& command_queue : m_command_queue)
    {
        command_queue.push_back([light, light_index](int buffer_index, PvpScene& scene) {
            void* light_base = scene.m_point_lights_gpu.get_buffer(buffer_index).get_allocation_info().pMappedData;

            std::span point_lights(reinterpret_cast<PointLight*>(reinterpret_cast<char*>(light_base) + 16u), max_direction_lights);
            point_lights[light_index] = light;
        });
    }
}

uint32_t pvp::PvpScene::add_direction_light(const DirectionLight& light)
{
    for (std::vector<std::function<void(int, PvpScene&)>>& command_queue : m_command_queue)
    {
        command_queue.push_back([light](int buffer_index, PvpScene& scene) {
            void*     light_base = scene.m_directonal_lights_gpu.get_buffer(buffer_index).get_allocation_info().pMappedData;
            uint32_t  light_index = (++*static_cast<uint32_t*>(light_base));
            std::span point_lights(reinterpret_cast<DirectionLight*>(reinterpret_cast<char*>(light_base) + 16u), max_direction_lights);
            point_lights[light_index - 1] = light;
        });
    }

    return 0;
}

void pvp::PvpScene::change_direction_light(uint32_t light_index, const DirectionLight& light)
{
    for (std::vector<std::function<void(int, PvpScene&)>>& command_queue : m_command_queue)
    {
        command_queue.push_back([light, light_index](int buffer_index, PvpScene& scene) {
            void* light_base = scene.m_directonal_lights_gpu.get_buffer(buffer_index).get_allocation_info().pMappedData;

            std::span direction_lights(reinterpret_cast<DirectionLight*>(reinterpret_cast<char*>(light_base) + 16u), max_direction_lights);
            direction_lights[light_index] = light;
        });
    }
}

void pvp::PvpScene::update()
{
    ZoneScoped;
    static auto start_time = std::chrono::high_resolution_clock::now();
    static auto last_time = std::chrono::high_resolution_clock::now();
    const auto  current_time = std::chrono::high_resolution_clock::now();
    const float delta_time = std::chrono::duration<float>(current_time - last_time).count();
    const float time = std::chrono::duration<float>(current_time - start_time).count();
    last_time = current_time;

    m_camera.update(delta_time);

    m_scene_globals = SceneGlobals{
        m_camera.get_view_matrix(),
        m_camera.get_projection_matrix(),
        m_camera.get_position(),
        m_camera.get_cone(),
    };

    // PointLight light = { glm::vec4(std::sin(time), 1.0f, std::cos(time), 0.0), { 1.0f, 0.5f, 0, 1 }, 150 };
    // change_point_light(0, light);

    // static bool key_pressed_last{};
    // bool        key_pressed = glfwGetKey(m_context.window_surface->get_window(), GLFW_KEY_SPACE) == GLFW_PRESS;

    // if (key_pressed && !key_pressed_last)
    // {
    // m_direction_light.intensity = m_direction_light.intensity > 5.0f ? 0.0f : 10.0f;
    // change_direction_light(0, m_direction_light);
    // }
    // key_pressed_last = key_pressed;

    // ImGui::ShowDemoWindow();

    if (ImGui::Begin("Debug"))
    {
        ImGui::Text("fps: %f", 1.0f / delta_time);
        ImGui::Text("Camera pos: x:%f, y:%f, z:%f", m_camera.get_position().x, m_camera.get_position().y, m_camera.get_position().z);

        if (ImGui::Button("Add light"))
        {
            add_point_light(PointLight(glm::vec4(m_camera.get_position(), 1.0), glm::vec4(1.0, 1.0, 1.0, 1.0), 1.0f));
        }

        ImGui::Separator();
        // TODO: Pure cringe. Have a copy of the lights on the cpu please.
        void* light_base = (m_point_lights_gpu.get_buffer(0).get_allocation_info().pMappedData);

        for (uint32_t i = 0; i < *static_cast<uint32_t*>(light_base); ++i)
        {
            ImGui::PushID(i);
            std::span  point_lights(reinterpret_cast<PointLight*>(reinterpret_cast<char*>(light_base) + 16u), max_direction_lights);
            PointLight light_pointing = point_lights[i];
            if (ImGui::DragFloat3("Position", &light_pointing.position.x, 0.1))
            {
                change_point_light(i, light_pointing);
            }
            if (ImGui::ColorEdit4("Color", &light_pointing.color.x))
            {
                change_point_light(i, light_pointing);
            }
            if (ImGui::DragFloat("intensity", &light_pointing.intensity))
            {
                change_point_light(i, light_pointing);
            }
            ImGui::Separator();
            ImGui::PopID();
        }

        ImGui::Text("TASK_SHADER_INVOCATIONS: %i", m_context.invocation_count[0]);
        ImGui::Text("MESH_SHADER_INVOCATIONS: %i", m_context.invocation_count[1]);
    }
    ImGui::End();
}

void pvp::PvpScene::update_render(const FrameContext& frame_context)
{
    auto& commands_buffer = m_command_queue[frame_context.buffer_index];
    for (std::function<void(int, PvpScene&)>& func : commands_buffer)
    {
        func(frame_context.buffer_index, *this);
    }
    commands_buffer.clear();

    // while (!commands_buffer.empty())
    // {
    //     commands_buffer.front()(frame_context.buffer_index, *this);
    //     commands_buffer.pop_front();
    // }

    m_scene_globals_gpu.update(frame_context.buffer_index, m_scene_globals);
}
