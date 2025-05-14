#pragma once
#include <Context/Context.h>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
namespace pvp
{
    class Camera final
    {
    public:
        explicit Camera(const Context& context);
        void update(float delta_time);

        glm::mat4x4 get_view_matrix() const
        {
            return m_view;
        };
        glm::mat4x4 get_perspective_matrix() const
        {
            return m_perspective;
        };

    private:
        const Context& m_context;
        glm::mat4x4    m_perspective{};
        glm::mat4x4    m_view{};
        glm::vec3      m_position{ 0, 0, 0 };
        glm::vec3      m_front{ 0, 0, 1 };
        glm::vec3      m_camera_up = glm::vec3{ 0, 1.0f, 0.0f };

        float m_yaw{ 0 };
        float m_pitch{ 0 };
        float m_speed{ 1 };
        float m_sensitivity{ 0.4f };

        double m_prev_mouse_x{};
        double m_prev_mouse_y{};
    };
} // namespace pvp
