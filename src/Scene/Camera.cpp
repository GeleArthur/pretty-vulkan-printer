#include "Camera.h"

#include "imgui.h"

#include <GlfwToRender.h>
#include <Renderer/Swapchain.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
pvp::Camera::Camera(const Context& context)
    : m_context(context)
{
}
void pvp::Camera::update(float delta_time)
{
    m_projection = glm::perspective(glm::radians(fov_angle), get_screen_ratio(), near_plane, far_plane);
    m_projection[1][1] *= -1;

    double xpos{};
    double ypos{};
    bool   pressed{};

    std::array<int, GLFW_KEY_LAST> keys{};
    {
        std::scoped_lock const lock{ m_context.gtfw_to_render->lock };
        xpos = m_context.gtfw_to_render->mouse_pos_x;
        ypos = m_context.gtfw_to_render->mouse_pos_y;
        pressed = m_context.gtfw_to_render->mouse_down[0];
        keys = m_context.gtfw_to_render->keys_pressed;
    }

    if (pressed && !ImGui::GetIO().WantCaptureMouse)
    {
        float xoffset = xpos - m_prev_mouse_x;
        float yoffset = m_prev_mouse_y - ypos;

        xoffset *= m_sensitivity;
        yoffset *= m_sensitivity;

        m_yaw += xoffset;
        m_pitch += yoffset;
    }

    m_prev_mouse_x = xpos;
    m_prev_mouse_y = ypos;

    if (!ImGui::GetIO().WantCaptureKeyboard)
    {
        glm::vec3 direction;
        direction.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        direction.y = sin(glm::radians(m_pitch));
        direction.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        m_front = glm::normalize(direction);

        if (keys[GLFW_KEY_W] == GLFW_PRESS)
            m_position += m_speed * m_front * delta_time;
        if (keys[GLFW_KEY_S] == GLFW_PRESS)
            m_position -= m_speed * m_front * delta_time;
        if (keys[GLFW_KEY_A] == GLFW_PRESS)
            m_position -= glm::normalize(glm::cross(m_front, m_camera_up)) * m_speed * delta_time;
        if (keys[GLFW_KEY_D] == GLFW_PRESS)
            m_position += glm::normalize(glm::cross(m_front, m_camera_up)) * m_speed * delta_time;

        m_view = glm::lookAt(m_position, m_position + m_front, m_camera_up);
    }
}
const pvp::FrustumCone& pvp::Camera::get_cone()
{
    if (update_frustum)
    {
        m_frustum_cone.tip = m_position;
        m_frustum_cone.direction = m_front;
        m_frustum_cone.height = 200.0f;             // far clip from perspective matrix
        m_frustum_cone.angle = glm::radians(45.0f); // Guessing fov
    }

    return m_frustum_cone;
}
const pvp::RadarCull& pvp::Camera::get_radar_cull()
{
    float tangent = std::tanf(glm::radians(fov_angle));
    m_radar_cull.far_plane = far_plane;
    m_radar_cull.near_plane = near_plane;
    m_radar_cull.tang = tangent;
    m_radar_cull.ratio = get_screen_ratio();
    m_radar_cull.sphere_factor_y = 1.0f / std::cosf(glm::radians(fov_angle));
    m_radar_cull.sphere_factor_x = 1.0f / std::cosf(std::atan(m_radar_cull.tang * m_radar_cull.ratio));

    m_radar_cull.camera_z = m_front;
    m_radar_cull.camera_x = glm::cross(m_front, m_camera_up);
    m_radar_cull.camera_y = glm::cross(m_radar_cull.camera_z, m_radar_cull.camera_x);
    return m_radar_cull;
}
const float pvp::Camera::get_screen_ratio()
{
    return static_cast<float>(m_context.swapchain->get_swapchain_extent().width) / static_cast<float>(m_context.swapchain->get_swapchain_extent().height);
}