#include "PVPScene.h"
#include "DescriptorSets/CommonDescriptorLayouts.h"
#include "ModelData.h"

#include <DestructorQueue.h>
#include <VulkanExternalFunctions.h>
#include <imgui.h>
#include <stb_image.h>
#include <Buffer/BufferBuilder.h>
#include <CommandBuffer/CommandPool.h>
#include <Context/Device.h>
#include <Debugger/Gizmos.h>
#include <DescriptorSets/DescriptorLayoutCreator.h>
#include <DescriptorSets/DescriptorLayoutBuilder.h>
#include <DescriptorSets/DescriptorSetBuilder.h>
#include <GLFW/glfw3.h>
#include <GraphicsPipeline/Vertex.h>
#include <Image/ImageBuilder.h>
#include <Image/SamplerBuilder.h>
#include <VMAAllocator/VmaAllocator.h>
#include <assimp/material.h>
#include <numeric>
#include <Debugger/debugger.h>
#include <glm/gtx/rotate_vector.hpp>
#include <spdlog/spdlog.h>
#include <tracy/Tracy.hpp>
#include <glm/glm.hpp>
#include <GraphicsPipeline/ShaderLoader.h>
#include <assimp/cimport.h>

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
        .set_layout(m_context.descriptor_creator->get_layout()
                        .add_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
                        .add_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
                        .set_tag(DiscriptorTag::lights)
                        .get())
        .bind_uniform_buffer(0, m_point_lights_gpu)
        .bind_uniform_buffer(1, m_directonal_lights_gpu)
        .build(m_context, m_point_descriptor);

    m_context.descriptor_creator->get_layout()
        .add_flag(0)
        .add_binding(VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_MESH_BIT_EXT)
        .add_flag(VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT)
        .add_binding(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_MESH_BIT_EXT, 300)
        .set_tag(DiscriptorTag::bindless_textures)
        .get();

    m_direction_light = DirectionLight{ { 0.557f, -0.557f, -0.557f, 0 }, { 1, 1, 1, 1.0f }, 10 };

    add_point_light(PointLight{ { 3, 0.1, 0, 0 }, { 1, 0, 0, 1.0f }, 100 });
    add_point_light(PointLight{ { 10, 2, -0.25f, 0 }, { 0, 1, 0, 1.0f }, 500 });
    add_direction_light(m_direction_light);

    scan_folder();
    ShaderLoader::init();
}

pvp::PvpScene::~PvpScene()
{
    unload_scenes();
    m_shadered_sampler.destroy(m_context.device->get_device());
}

void pvp::PvpScene::load_scene(const std::filesystem::path& path)
{
    ZoneScoped;

    const std::optional<LoadedScene> scene_optional = load_scene_cpu(path);
    if (!scene_optional.has_value())
        return;

    unload_scenes();
    const LoadedScene& loaded_scene = scene_optional.value();

    const CommandPool     cmd_pool_transfer_buffers = CommandPool(m_context, *m_context.queue_families->get_queue_family(VK_QUEUE_TRANSFER_BIT, false), VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
    const VkCommandBuffer cmd = cmd_pool_transfer_buffers.begin_buffer();
    DestructorQueue       transfer_deleter{};

    load_default_textures(transfer_deleter, cmd);
    load_textures(loaded_scene, transfer_deleter, cmd);

    big_buffer_generation(loaded_scene, transfer_deleter, cmd);

    m_gpu_models.reserve(m_gpu_models.size() + loaded_scene.models.size());

    for (const ModelData& cpu_model : loaded_scene.models)
    {
        ZoneScopedN("Model");
        // transfer_to_gpu(cpu_model.vertices, m_gpu_vertices, offset_vertex);
        // transfer_to_gpu(cpu_model.indices, m_gpu_indices, offset_indices);

        Model& gpu_model = m_gpu_models.emplace_back();

        auto transfer_to_gpu = [this, &transfer_deleter, &cmd]<typename T>(const std::span<T> data, Buffer& gpu_buffer, VkBufferUsageFlags usage, const std::string& name = "") {
            Buffer transfer_buffer{};
            BufferBuilder()
                .set_size(data.size_bytes())
                .set_usage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
                .set_flags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT)
                .set_memory_usage(VMA_MEMORY_USAGE_AUTO_PREFER_HOST)
                .build(m_context.allocator->get_allocator(), transfer_buffer);

            transfer_buffer.copy_data_into_buffer(data);
            transfer_deleter.add_to_queue([transfer_buffer] {
                transfer_buffer.destroy();
            });

            BufferBuilder()
                .set_size(data.size_bytes())
                .set_usage(usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT)
                .set_memory_usage(VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE)
                .build(m_context.allocator->get_allocator(), gpu_buffer);
            gpu_buffer.copy_from_buffer(cmd, transfer_buffer);

            debugger::add_object_name(m_context.device, gpu_buffer.get_buffer(), name);
        };

        transfer_to_gpu(std::span(cpu_model.vertices), gpu_model.vertex_data, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, "Mesh vertex data");

        // Index loading
        transfer_to_gpu(std::span(cpu_model.indices), gpu_model.index_data, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, "indcies");
        gpu_model.index_count = cpu_model.indices.size();
        gpu_model.meshlet_count = cpu_model.meshlets.size();

        // meshletes loading
        transfer_to_gpu(std::span(cpu_model.meshlets), gpu_model.meshlet_buffer, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, "Meshlets");
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

        gpu_model.material.diffuse_texture_index = cpu_model.diffuse_path.empty() ?
            1 :
            std::ranges::find_if(m_gpu_textures, [&](StaticImage& image) { return cpu_model.diffuse_path == image.get_name(); }) - m_gpu_textures.begin();
        gpu_model.material.normal_texture_index = cpu_model.normal_path.empty() ?
            2 :
            std::ranges::find_if(m_gpu_textures, [&](StaticImage& image) { return cpu_model.normal_path == image.get_name(); }) - m_gpu_textures.begin();
        gpu_model.material.metalness_texture_index = cpu_model.metallic_path.empty() ?
            0 :
            std::ranges::find_if(m_gpu_textures, [&](StaticImage& image) { return cpu_model.metallic_path == image.get_name(); }) - m_gpu_textures.begin();

        // auto get_address = [&](VkBuffer buffer) -> VkDeviceAddress {
        //     VkBufferDeviceAddressInfo address_info{ .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR, .pNext = nullptr, .buffer = buffer };
        //     return vkGetBufferDeviceAddress(m_context.device->get_device(), &address_info);
        // };

        // gpu_model.ptr_to_buffers.vertex_data = get_address(gpu_model.vertex_data.get_buffer());
        // gpu_model.ptr_to_buffers.meshlet_data = get_address(gpu_model.meshlet_buffer.get_buffer());
        // gpu_model.ptr_to_buffers.meshlet_vertices_data = get_address(gpu_model.meshlet_vertices_buffer.get_buffer());
        // gpu_model.ptr_to_buffers.meshlet_triangle_data = get_address(gpu_model.meshlet_triangles_buffer.get_buffer());
        // gpu_model.ptr_to_buffers.meshlet_sphere_bounds_data = get_address(gpu_model.meshlet_sphere_bounds_buffer.get_buffer());
    }

    {
        BufferBuilder()
            .set_size(loaded_scene.models.size() * sizeof(MaterialTransform))
            .set_usage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
            .set_memory_usage(VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE)
            .build(m_context.allocator->get_allocator(), m_gpu_matrix);
        m_scene_destructor_queue.add_to_queue([&] { m_gpu_matrix.destroy(); });

        std::vector<MaterialTransform> all_matricies;
        all_matricies.reserve(loaded_scene.models.size());
        for (const Model& model : m_gpu_models)
        {
            all_matricies.push_back(model.material);
        }
        m_gpu_matrix.copy_data_from_tmp_buffer(m_context, cmd, std::span(all_matricies), transfer_deleter);
    }

    cmd_pool_transfer_buffers.end_buffer(cmd);
    transfer_deleter.destroy_and_clear();
    cmd_pool_transfer_buffers.destroy();

    DescriptorSetBuilder()
        .set_layout(m_context.descriptor_creator->get_layout().from_tag(DiscriptorTag::bindless_textures).get())
        .bind_sampler(0, m_shadered_sampler)
        .bind_image_array(1, m_gpu_textures)
        .build(m_context, m_all_textures);

    build_draw_calls();

    DescriptorSetBuilder{}
        .set_layout(m_context.descriptor_creator->get_layout()
                        .add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT)
                        .add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT)
                        .add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT)
                        .add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT)
                        .add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT)
                        .add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT)
                        .add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT)
                        .add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT)
                        .set_tag(DiscriptorTag::big_buffers)
                        .get())
        .bind_buffer_ssbo(0, get_indirect_draw_calls())
        .bind_buffer_ssbo(1, get_matrix_buffer())
        .bind_buffer_ssbo(2, get_meshlets_buffer())
        .bind_buffer_ssbo(3, get_all_vertex_buffer())
        .bind_buffer_ssbo(4, get_meshlets_vertices_buffer())
        .bind_buffer_ssbo(5, get_meshlets_triangles_buffer())
        .bind_buffer_ssbo(6, get_meshlets_sphere_bounds_buffer())
        .bind_buffer_ssbo(7, get_pointers())
        .build(m_context, m_indirect_descriptor);

    DescriptorSetBuilder{}
        .set_layout(m_context.descriptor_creator->get_layout()
                        .add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT)
                        .add_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT)
                        .set_tag(DiscriptorTag::pointers)
                        .get())
        .bind_buffer_ssbo(0, m_gpu_indirect_draw_calls)
        .bind_buffer_ssbo(1, m_pointers)
        .build(m_context, m_indirect_descriptor_ptr);

    // m_scene_globals_gpu = UniformBuffer{};
    // m_point_lights_gpu = UniformBuffer(16 + sizeof(PointLight) * max_point_lights, context.allocator->get_allocator());
    // m_directonal_lights_gpu = UniformBuffer(16 + sizeof(DirectionLight) * max_direction_lights, context.allocator->get_allocator());
}
void pvp::PvpScene::unload_scenes()
{
    vkDeviceWaitIdle(m_context.device->get_device());
    m_scene_destructor_queue.destroy_and_clear();

    for (const Model& model : m_gpu_models)
    {
        model.vertex_data.destroy();
        model.index_data.destroy();
        model.meshlet_buffer.destroy();
        model.meshlet_vertices_buffer.destroy();
        model.meshlet_triangles_buffer.destroy();
        model.meshlet_sphere_bounds_buffer.destroy();

        model.meshlet_descriptor_set.destroy();
    }
    m_gpu_models.clear();

    for (const StaticImage& gpu_texture : m_gpu_textures)
    {
        gpu_texture.destroy(m_context);
    }
    m_gpu_textures.clear();
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

void pvp::PvpScene::scan_folder()
{
    auto scan = [&](const std::string& path) {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(path))
        {
            if (entry.is_regular_file())
            {
                auto ext = entry.path().extension().string();
                if (ext == ".glb" || ext == ".obj" || ext == ".gltf" || ext == ".fbx")
                {
                    m_scene_files.push_back(entry.path().string());
                }
            }
        }
    };
    scan("resources");
    scan("../intelsponza");
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

    m_counted_up_delta_time.push_back(delta_time);
    m_result_timer -= delta_time;
    if (m_result_timer < 0)
    {
        m_result_timer = 1;
        if (m_counted_up_delta_time.size() > 2)
        {
            std::ranges::sort(m_counted_up_delta_time);
            m_counted_up_delta_time.erase(m_counted_up_delta_time.begin());
            m_counted_up_delta_time.erase(m_counted_up_delta_time.end() - 2);
            float total = std::accumulate(m_counted_up_delta_time.begin(), m_counted_up_delta_time.end(), 0.0f, std::plus());
            m_result_delta_time = total / m_counted_up_delta_time.size();
            m_counted_up_delta_time.clear();
        }
    }

    gizmos::clear();

    m_camera.update(delta_time);

    const FrustumCone frustum_cone = m_camera.get_cone();
    m_scene_globals = SceneGlobals{
        m_camera.get_view_matrix(),
        m_camera.get_projection_matrix(),
        m_camera.get_position(),
        frustum_cone,
        m_camera.get_projection_matrix() * m_camera.get_view_matrix()
    };
    gizmos::draw_cone(frustum_cone.tip, frustum_cone.height, frustum_cone.direction, frustum_cone.angle);

    if (ImGui::Begin("Debug"))
    {
        ImGui::Text("fps: %f", 1.0f / delta_time);
        ImGui::Text("ms: %f", delta_time * 1000.0f);
        ImGui::Text("avarage fps: %f", 1.0f / m_result_delta_time);
        ImGui::Text("avarage ms: %f", m_result_delta_time * 1000.0f);
        ImGui::Text("Camera pos: x:%f, y:%f, z:%f", m_camera.get_position().x, m_camera.get_position().y, m_camera.get_position().z);

        ImGui::Separator();

        static int selectedSceneIndex = 0;

        if (ImGui::Button("Rescan"))
        {
            scan_folder();
            selectedSceneIndex = 0;
        }
        ImGui::Text("Select Scene:");
        if (ImGui::BeginCombo("##scenes", m_scene_files.empty() ? "No scenes" : m_scene_files[selectedSceneIndex].c_str()))
        {
            for (int i = 0; i < m_scene_files.size(); i++)
            {
                bool isSelected = (selectedSceneIndex == i);
                if (ImGui::Selectable(m_scene_files[i].c_str(), isSelected))
                {
                    selectedSceneIndex = i;
                }
                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        if (ImGui::Button("Load Scene", ImVec2(120, 0)))
        {
            if (!m_scene_files.empty())
            {
                load_scene(std::filesystem::path(m_scene_files[selectedSceneIndex]));
            }
        }

        ImGui::Separator();

        if (ImGui::Button("Add light"))
        {
            add_point_light(PointLight(glm::vec4(m_camera.get_position(), 1.0), glm::vec4(1.0, 1.0, 1.0, 1.0), 1.0f));
        }

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
            ImGui::PopID();
        }

        ImGui::Text("MESH_SHADER_INVOCATIONS: %llu", m_context.invocation_count[0]);
        ImGui::Text("TASK_SHADER_INVOCATIONS: %llu", m_context.invocation_count[1]);

        ImGui::Checkbox("Update frustom", &m_camera.update_frustum);
        ImGui::Checkbox("Enable spheres", &m_spheres_enabled);

        constexpr std::array<const char*, 3> render_modes{ "CPU", "GPU Indirect", "GPU Indirect ptr" };
        ImGui::Combo("RenderMode", reinterpret_cast<int*>(&m_render_mode), render_modes.data(), render_modes.size());

        ImGui::Checkbox("Enable meshlets", &m_meshlets_enabled);
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
void pvp::PvpScene::generate_mipmaps(VkCommandBuffer cmd, StaticImage& gpu_image, uint32_t width, uint32_t height)
{
    for (uint32_t i = 1; i < gpu_image.get_mipmap_levels(); ++i)
    {
        VkImageBlit2 region{
            .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
            .srcSubresource = VkImageSubresourceLayers{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = i - 1,
                .baseArrayLayer = 0,
                .layerCount = 1 },
            .srcOffsets = { VkOffset3D{ 0, 0, 0 }, VkOffset3D{ static_cast<int32_t>(width >> (i - 1)), static_cast<int32_t>(height >> (i - 1)), 1 } },
            .dstSubresource = VkImageSubresourceLayers{ .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = i, .baseArrayLayer = 0, .layerCount = 1 },
            .dstOffsets = { VkOffset3D{ 0, 0, 0 }, VkOffset3D{ static_cast<int32_t>(width >> i), static_cast<int32_t>(height >> i), 1 } },
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
}
void pvp::PvpScene::load_textures(const LoadedScene& loaded_scene, DestructorQueue& transfer_deleter, VkCommandBuffer cmd)
{
    ZoneScoped;
    m_gpu_textures.reserve(m_gpu_textures.size() + loaded_scene.textures.size());

    for (const TextureData& texture : loaded_scene.textures)
    {
        ZoneScopedN("Texture");
        ZoneTextF(texture.name.c_str());
        // VkDeviceSize image_size = texture.width * texture.height * texture.channels;

        Buffer staging_buffer{};
        BufferBuilder()
            .set_size(texture.pixels.size() * sizeof(uint8_t))
            .set_usage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
            .set_flags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT)
            .build(m_context.allocator->get_allocator(), staging_buffer);

        transfer_deleter.add_to_queue([=] { staging_buffer.destroy(); });
        staging_buffer.copy_data_into_buffer(std::span(texture.pixels));

        StaticImage gpu_image;
        ImageBuilder()
            .set_name(texture.name)
            .set_format(texture.format)
            .set_usage(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
            .set_size({ .width = texture.width, .height = texture.height })
            .set_memory_usage(VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE)
            .set_aspect_flags(VK_IMAGE_ASPECT_COLOR_BIT)
            .set_use_mipmap(texture.generate_mip_maps)
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
        generate_mipmaps(cmd, gpu_image, texture.width, texture.height);
        gpu_image.transition_layout(cmd,
                                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                    VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                                    VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                                    VK_ACCESS_2_TRANSFER_READ_BIT,
                                    VK_ACCESS_2_SHADER_READ_BIT);

        m_gpu_textures.push_back(std::move(gpu_image));
    }
}

void pvp::PvpScene::load_default_textures(DestructorQueue& transfer_deleter, VkCommandBuffer cmd)
{
    auto create_default_texture = [&](const std::array<uint8_t, 4>& data, const std::string& name) {
        Buffer staging_buffer{};
        BufferBuilder()
            .set_size(4 * sizeof(uint8_t))
            .set_usage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
            .set_flags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT)
            .build(m_context.allocator->get_allocator(), staging_buffer);

        transfer_deleter.add_to_queue([=] { staging_buffer.destroy(); });
        staging_buffer.copy_data_into_buffer(std::span(data.cbegin(), data.cend()));

        StaticImage gpu_image;
        ImageBuilder()
            .set_name(name.c_str())
            .set_format(VK_FORMAT_R8G8B8A8_UNORM)
            .set_usage(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
            .set_size({ .width = 1, .height = 1 })
            .set_memory_usage(VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE)
            .set_aspect_flags(VK_IMAGE_ASPECT_COLOR_BIT)
            .set_use_mipmap(false)
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
                                    VK_ACCESS_2_TRANSFER_READ_BIT,
                                    VK_ACCESS_2_SHADER_READ_BIT);

        m_gpu_textures.push_back(std::move(gpu_image));
    };

    create_default_texture({ 0u, 0u, 0u, 255u }, "Default: Black");
    create_default_texture({ 255u, 255u, 255u, 255u }, "Default: White");
    create_default_texture({ 128u, 128u, 255u, 255u }, "Default: normal");
}

void pvp::PvpScene::big_buffer_generation(const LoadedScene& loaded_scene, DestructorQueue& transfer_deleter, VkCommandBuffer cmd)
{
    ZoneScoped;
    auto load_data_into_big_buffer = [&](auto ModelData::* member_ptr, Buffer& buffer) {
        using VectorType = std::decay_t<decltype(std::declval<ModelData>().*member_ptr)>::value_type;

        uint32_t const total_count = std::accumulate(
            loaded_scene.models.cbegin(),
            loaded_scene.models.cend(),
            0u,
            [member_ptr](uint32_t start, const ModelData& model) {
                return static_cast<uint32_t>(start + (model.*member_ptr).size());
            });

        BufferBuilder()
            .set_size(total_count * sizeof(VectorType))
            .set_usage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
            .set_memory_usage(VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE)
            .build(m_context.allocator->get_allocator(), buffer);
        m_scene_destructor_queue.add_to_queue([buffer] { buffer.destroy(); });

        std::vector<VectorType> all_data;
        all_data.reserve(total_count);

        for (const ModelData& model : loaded_scene.models)
        {
            for (const auto& element : (model.*member_ptr))
            {
                all_data.push_back(element);
            }
        }

        buffer.copy_data_from_tmp_buffer(m_context, cmd, std::span(all_data), transfer_deleter);
    };

    load_data_into_big_buffer(&ModelData::vertices, m_gpu_vertices);
    // load_data_into_big_buffer(&ModelData::indices, m_gpu_indices);

    // load_data_into_big_buffer(&ModelData::meshlet_vertices, m_gpu_meshlets_vertices);
    load_data_into_big_buffer(&ModelData::meshlet_triangles, m_gpu_meshlets_triangles);
    load_data_into_big_buffer(&ModelData::meshlet_sphere_bounds, m_gpu_meshlets_sphere_bounds);

    // load_data_into_big_buffer(&ModelData::meshlets, m_gpu_meshlets);

    // Generate vertex indices
    {
        uint32_t const total_count = std::accumulate(
            loaded_scene.models.cbegin(),
            loaded_scene.models.cend(),
            0u,
            [](uint32_t start, const ModelData& model) {
                return static_cast<uint32_t>(start + model.meshlet_vertices.size());
            });

        BufferBuilder()
            .set_size(total_count * sizeof(uint32_t))
            .set_usage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
            .set_memory_usage(VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE)
            .build(m_context.allocator->get_allocator(), m_gpu_meshlets_vertices);
        m_scene_destructor_queue.add_to_queue([&] { m_gpu_meshlets_vertices.destroy(); });

        std::vector<uint32_t> all_data;
        all_data.reserve(total_count);

        uint32_t meshlet_vertex_global_count{};
        for (int i = 0; i < loaded_scene.models.size(); ++i)
        {
            for (int j = 0; j < loaded_scene.models[i].meshlet_vertices.size(); ++j)
            {
                all_data.push_back(meshlet_vertex_global_count + loaded_scene.models[i].meshlet_vertices[j]);
            }
            meshlet_vertex_global_count += loaded_scene.models[i].vertices.size();
        }

        m_gpu_meshlets_vertices.copy_data_from_tmp_buffer(m_context, cmd, std::span(all_data), transfer_deleter);
    }
    // Generate meshlets offsets
    {
        uint32_t const total_count = std::accumulate(
            loaded_scene.models.cbegin(),
            loaded_scene.models.cend(),
            0u,
            [](uint32_t start, const ModelData& model) {
                return static_cast<uint32_t>(start + (model.meshlets).size());
            });

        BufferBuilder()
            .set_size(total_count * sizeof(meshopt_Meshlet))
            .set_usage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
            .set_memory_usage(VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE)
            .build(m_context.allocator->get_allocator(), m_gpu_meshlets);
        m_scene_destructor_queue.add_to_queue([&] { m_gpu_meshlets.destroy(); });

        std::vector<meshopt_Meshlet> all_data;
        all_data.reserve(total_count);

        uint32_t triangle_global_count{};
        uint32_t vertex_global_count{};
        for (int i = 0; i < loaded_scene.models.size(); ++i)
        {
            for (int j = 0; j < loaded_scene.models[i].meshlets.size(); ++j)
            {
                all_data.emplace_back(vertex_global_count,
                                      triangle_global_count,
                                      loaded_scene.models[i].meshlets[j].vertex_count,
                                      loaded_scene.models[i].meshlets[j].triangle_count);

                vertex_global_count += loaded_scene.models[i].meshlets[j].vertex_count;
                triangle_global_count += loaded_scene.models[i].meshlets[j].triangle_count * 3;
            }
        }

        m_gpu_meshlets.copy_data_from_tmp_buffer(m_context, cmd, std::span(all_data), transfer_deleter);
    }
}

void pvp::PvpScene::build_draw_calls()
{
    ZoneScoped;
    const std::vector<Model>& models = get_models();
    BufferBuilder{}
        .set_size(sizeof(DrawCommandIndirect) * models.size())
        .set_memory_usage(VMA_MEMORY_USAGE_AUTO)
        .set_usage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT)
        .set_flags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT)
        .build(m_context.allocator->get_allocator(), m_gpu_indirect_draw_calls);
    m_scene_destructor_queue.add_to_queue([&] { m_gpu_indirect_draw_calls.destroy(); });

    DrawCommandIndirect* buffer_array = static_cast<DrawCommandIndirect*>(m_gpu_indirect_draw_calls.get_allocation_info().pMappedData);
    uint32_t             meshlet_offset{};
    for (int i = 0; i < models.size(); ++i)
    {
        uint32_t thread_group_count_x = models[i].meshlet_count / mesh_let_count + 1;
        buffer_array[i] = DrawCommandIndirect{
            thread_group_count_x,
            1,
            1,
            meshlet_offset,
            models[i].meshlet_count
        };
        meshlet_offset += models[i].meshlet_count;
    }

    BufferBuilder{}
        .set_size(sizeof(MeshletsBuffers) * models.size())
        .set_flags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT)
        .set_memory_usage(VMA_MEMORY_USAGE_AUTO_PREFER_HOST)
        .set_usage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
        .build(m_context.allocator->get_allocator(), m_pointers);

    auto get_address = [&](VkBuffer buffer) -> VkDeviceAddress {
        VkBufferDeviceAddressInfo address_info{ .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR, .pNext = nullptr, .buffer = buffer };
        return vkGetBufferDeviceAddress(m_context.device->get_device(), &address_info);
    };

    for (int i = 0; i < models.size(); ++i)
    {
        static_cast<MeshletsBuffers*>(m_pointers.get_allocation_info().pMappedData)[i] =
            MeshletsBuffers{
                .vertex_data = get_address(models[i].vertex_data.get_buffer()),
                .meshlet_data = get_address(models[i].meshlet_buffer.get_buffer()),
                .meshlet_vertices_data = get_address(models[i].meshlet_vertices_buffer.get_buffer()),
                .meshlet_triangle_data = get_address(models[i].meshlet_triangles_buffer.get_buffer()),
                .meshlet_sphere_bounds_data = get_address(models[i].meshlet_sphere_bounds_buffer.get_buffer()),
            };
    }

    vmaFlushAllocation(m_context.allocator->get_allocator(), m_pointers.get_allocation(), 0, sizeof(MeshletsBuffers) * models.size());

    m_scene_destructor_queue.add_to_queue([&] { m_pointers.destroy(); });
}
