#pragma once
#include "Camera.h"
#include "ModelData.h"

#include <DestructorQueue.h>
#include <deque>
#include <filesystem>
#include <vector>
#include <Buffer/Buffer.h>
#include <Context/Context.h>
#include <DescriptorSets/DescriptorSets.h>
#include <Image/Sampler.h>
#include <Image/StaticImage.h>
#include <UniformBuffers/UniformBuffer.h>
#include <glm/mat4x4.hpp>
#include <glm/detail/func_packing_simd.inl>

namespace pvp
{
    struct Sampler;

    struct MaterialTransform
    {
        glm::mat4x4 transform;
        uint32_t    diffuse_texture_index;
        uint32_t    normal_texture_index;
        uint32_t    metalness_texture_index;
    };

    struct Model
    {
        Buffer            vertex_data;
        Buffer            index_data;
        uint32_t          index_count;
        MaterialTransform material;

        // Meshlet data
        uint32_t       meshlet_count;
        DescriptorSets meshlet_descriptor_set;
        // Buffer         meshlet_vertex;
        Buffer meshlet_buffer;
        Buffer meshlet_vertices_buffer;
        Buffer meshlet_triangles_buffer;
        Buffer meshlet_sphere_bounds_buffer;
    };

    struct SceneGlobals
    {
        alignas(16) glm::mat4x4 camera_view;
        alignas(16) glm::mat4x4 camera_projection;
        alignas(16) glm::vec3 positon;
        alignas(16) FrustumCone cone;
    };

    struct alignas(16) PointLight
    {
        glm::vec4 position;
        glm::vec4 color;
        float     intensity;
    };

    struct alignas(16) DirectionLight
    {
        glm::vec4 direction;
        glm::vec4 color;
        float     intensity;
    };

    class PvpScene final
    {
    public:
        explicit PvpScene(Context& context);
        ~PvpScene();
        DISABLE_COPY(PvpScene);
        DISABLE_MOVE(PvpScene);

        void     load_scene(const std::filesystem::path& path);
        void     unload_scenes();
        uint32_t add_point_light(const PointLight& light);
        void     change_point_light(uint32_t light_index, const PointLight& light);
        uint32_t add_direction_light(const DirectionLight& light);
        void     change_direction_light(uint32_t light_index, const DirectionLight& light);

        void update();
        void update_render(const FrameContext& frame_context);

        const std::vector<Model>& get_models() const
        {
            return m_gpu_models;
        };
        const std::vector<StaticImage>& get_textures() const
        {
            return m_gpu_textures;
        };
        const UniformBuffer& get_scene_globals() const
        {
            return m_scene_globals_gpu;
        };
        const DescriptorSets& get_scene_descriptor() const
        {
            return m_scene_binding;
        }
        const DescriptorSets& get_textures_descriptor() const
        {
            return m_all_textures;
        }
        const DescriptorSets& get_light_descriptor() const
        {
            return m_point_descriptor;
        }
        const Buffer& get_all_vertex_buffer() const
        {
            return m_gpu_vertices;
        }
        const Buffer& get_indices_buffer() const
        {
            return m_gpu_indices;
        }
        const Buffer& get_matrix_buffer() const
        {
            return m_gpu_matrix;
        }
        const Buffer& get_meshlets_buffer() const
        {
            return m_gpu_meshlets;
        }
        const Buffer& get_meshlets_vertices_buffer() const
        {
            return m_gpu_meshlets_vertices;
        }
        const Buffer& get_meshlets_triangles_buffer() const
        {
            return m_gpu_meshlets_triangles;
        }
        const Buffer& get_meshlets_sphere_bounds_buffer() const
        {
            return m_gpu_meshlets_sphere_bounds;
        }

    private:
        void generate_mipmaps(VkCommandBuffer cmd, StaticImage& gpu_image, uint32_t width, uint32_t height);
        void load_textures(const LoadedScene& scene, DestructorQueue& transfer_deleter, VkCommandBuffer cmd);
        void big_buffer_generation(const LoadedScene& loaded_scene, DestructorQueue& transfer_deleter, VkCommandBuffer cmd);

        Context&                 m_context;
        std::vector<Model>       m_gpu_models;
        std::vector<StaticImage> m_gpu_textures;
        DestructorQueue          m_scene_destructor_queue;
        DescriptorSets           m_scene_binding;
        DescriptorSets           m_all_textures;
        DescriptorSets           m_point_descriptor;
        Sampler                  m_shadered_sampler;
        SceneGlobals             m_scene_globals;
        UniformBuffer            m_point_lights_gpu;
        UniformBuffer            m_directonal_lights_gpu;
        UniformBuffer            m_scene_globals_gpu;

        Buffer m_gpu_vertices;
        Buffer m_gpu_indices;
        Buffer m_gpu_matrix;

        Buffer m_gpu_meshlets;
        Buffer m_gpu_meshlets_vertices;
        Buffer m_gpu_meshlets_triangles;
        Buffer m_gpu_meshlets_sphere_bounds;

        std::vector<std::vector<std::function<void(int, PvpScene&)>>> m_command_queue;

        Camera         m_camera;
        DirectionLight m_direction_light{};

        constexpr static uint32_t max_point_lights{ 10u };
        constexpr static uint32_t max_direction_lights{ 10u };
    };
} // namespace pvp
